#include "../h/const.h"

#define MAX_PROGRAMS 5	/* kernel, mm, fs, init, fsck */
#define MIN_ARGC 3	/* minimum command arguments (build bootblok output_f) */
#define READ_UNIT 512	/* how big a chunk to read in */
#define SECTOR_SIZE 512
#define BOOTBLOK_SIZE 512L
#define READ 0	/* value 0 means ASCII read */
#define CLICK_SHIFT 4
#define DS_OFFSET 4
#define KERNEL_D_MAGIC 0x526F	/* refer to kernel/mpx88.s */

/* order of arguments of 'build' binary */
#define KERN 0
#define MM 1
#define FS 2
#define INIT 3
#define FSCK 4

/* information about the file header */
#define STRIP_HDR_SIZE 32	/* short form header size */
#define UNSTRIP_HDR_SIZE 48	/* long form header size*/
#define SEP_POS 1	/* tells where sep I & D bit is */
#define HDR_LEN 2	/* tells where header length is */
#define TEXT_POS 0	/* where is text size in header */
#define DATA_POS 1	/* where is data size in header */
#define BSS_POS 2	/* where is bss size in header */
#define SEP_ID_BIT 0x20	/* bit that tells if file is seperate I & D */

#define FSCK_OFFSET 1	/* index of sizes = sizes[num_programs - FSCK_OFFSET] */

int num_programs;	/* it can be up to 5 programs (kernel+mm+fs+init+fsck) */
int image; 	/* file descriptor used for output file */
int buf_bytes;	/* # bytes in buf(output buffer) at present and index for next input byte */
int cur_sector;	/* #sector to be written next */
char buf[SECTOR_SIZE];	/* buffer for output file */
char zeros[SECTOR_SIZE];	/* zeros for bss segment */

long core_size;	/* size of kernel+mm+fs+init */
long all_size;	/* size of all programs */

struct sizes {
	unsigned text_size;
	unsigned data_size;
	unsigned bss_size;
	int sep_id;	/* 1 if separate, 0 if not */
} sizes[MAX_PROGRAMS];

create_image(f)
char *f;
{
	image = creat(f, 0666);
	close(image);
	image = open(f, 2);
}

copy_boot(file_name)
char *file_name;
{
	/* copy the specified file to the output. Note that file has no header.
	 * All the bytes are copied, until end of file is hit.
	 */

	int fd, bytes_read;
	char inbuf[READ_UNIT];

	if ((fd = open(file_name, READ)) < 0)
		pexit("can't open ", file_name);

	do {
		bytes_read = read(fd, inbuf, READ_UNIT);
		if (bytes_read < 0)
			pexit("read error on file ", file_name); 
		if (bytes_read > 0)
			wr_out(inbuf, bytes_read);
	} while (bytes_read > 0);

	flush();
	close(fd);
}

copy_prog(num, file_name)
int num;
char *file_name;
{
	/* Open and read a file, copying it to output. First read the header 
	 * to get the text, data, and bss sizes. Write the text, data and bss to output.
	 * The sum of these three pieces must be padded upwards to a multiple of 16, if needs, 
	 * The individual pieces doesn't need to be multiples of 16 bytes. The total size must be
	 * less than 64K
	*/

	int fd, sepid, bytes_read, count;
	/* data type and bss type: http://en.wikipedia.org/wiki/Data_segment */
	unsigned text_bytes, data_bytes, bss_bytes, tot_bytes, remainder, click_filler;
	unsigned left_to_read; 
	char inbuf[READ_UNIT];

	if ((fd = open(file_name, READ)) < 0)
		pexit("can't open ", file_name);

	/* Read the header to see how big the segments are */
	read_header(fd, &sepid, &text_bytes, &data_bytes, &bss_bytes, file_name);
	tot_bytes = text_bytes + data_bytes + bss_bytes;
	remainder = tot_bytes % 16;
	click_filler = (remainder > 0 ? 16 - remainder : 0);
	bss_bytes += click_filler; /* pad upward */
	tot_bytes += click_filler;

	if (num < (num_programs - FSCK_OFFSET)) 
		core_size += tot_bytes;
	all_size += tot_bytes;

	/* record the size information in the table. it is needed for patch functions later in this file */
	sizes[num].text_size = text_bytes;
	sizes[num].data_size = data_bytes;
	sizes[num].bss_size = bss_bytes;
	sizes[num].sep_id = sepid;

	/* Print a message giving the program name and size, except for fsck */	
	if (num < num_programs)
		printf("%s  text=%5u  data=%5u  bss=%5u  tot=%5u  hex=%4x  %s\n",
			file_name, text_bytes, data_bytes, bss_bytes, tot_bytes,
			tot_bytes, (sizes[num].sep_id ? "Seperate I & D" : ""));

	/* read in the text and data segments, and copy them to output */
	/* TOKNOW: why write in 512bytes chunk */
	left_to_read = text_bytes + data_bytes;
	while (left_to_read > 0) {
		count = (left_to_read < READ_UNIT ? left_to_read : READ_UNIT);
		bytes_read = read(fd, inbuf, count);
		if (bytes_read < 0) 
			pexit("read error on file ", file_name);
		if (bytes_read > 0)
			wr_out(inbuf, bytes_read);
		left_to_read -= count;
	}

	/* read in the bss to output */
	while (bss_bytes > 0) {
		count = (bss_bytes < SECTOR_SIZE ? bss_bytes : SECTOR_SIZE);
		wr_out(zeros, count);	
		bss_bytes -= count;	
	}
	close(fd);
}

read_header(fd, sepid, text_bytes, data_bytes, bss_bytes, file_name)
int fd, *sepid;
unsigned *text_bytes, *data_bytes, *bss_bytes;
char *file_name;
{
	/* Read the header and check the magic number.  The standard header 
	 * consists of 8 longs, as follows:
	 *      0: 0x04100301L (combined I & D space) or 0x04200301L (separate I & D)
	 *      1: 0x00000020L (stripped file) or 0x00000030L (unstripped file)
	 *      2: size of text segments in bytes
	 *      3: size of initialized data segment in bytes
	 *      4: size of bss in bytes
	 *      5: 0x00000000L
	 *      6: total memory allocated to program (text, data and stack, combined)
	 *      7: 0x00000000L
	 * The longs are represented low-order byte first and high-order byte last.
	 * The first byte of the header is always 0x01, followed by 0x03.
	 * The header is followed directly by the text and data segments, whose sizes
	 * are given in the header.
	 * http://en.wikipedia.org/wiki/File_format#Magic_number
	 */

	long head[12];
	unsigned short hd[4];
	int n, header_len;

	if ((n = read(fd, hd, 8)) != 8)
		pexit("[first 8 bytes]file header is too short: ", file_name);
	header_len = hd[HDR_LEN];	/* TOKNOW: why this is index 2? */
	if (header_len != STRIP_HDR_SIZE && header_len != UNSTRIP_HDR_SIZE)
		pexit("bad header length.File: ", file_name);

	*sepid = hd[SEP_POS] & SEP_ID_BIT;

	if (n = read(fd, head, header_len - 8) != header_len - 8)
		pexit("[rest of bytes]header is too short: ", file_name);

	*text_bytes = (unsigned) head[TEXT_POS];
	*data_bytes = (unsigned) head[DATA_POS];
	*bss_bytes = (unsigned) head[BSS_POS];
}

int get_byte(offset)
long offset;
{
	/* fetch one byte from the output file */
	char buff[SECTOR_SIZE];
	read_block((unsigned)(offset / SECTOR_SIZE), buff);
	return (buff[(unsigned)(offset % SECTOR_SIZE)] & 0377);
}

put_byte(offset, byte_value)
long offset;
int byte_value;
{
	char buff[SECTOR_SIZE];

	read_block((unsigned)(offset / SECTOR_SIZE), buff);
	buff[(unsigned)(offset % SECTOR_SIZE)] = byte_value;
	write_block((unsigned)(offset / SECTOR_SIZE), buff);
}

wr_out(buffer, bytes)
char buffer[READ_UNIT];
int bytes;
{
	/* Write bytes to the output buffer. This procedure must write 513-bytes block(one sector)-based */
	int room, count, count1;
	register char *dest, *src;

	/* copy the date to the output buffer */
	room = SECTOR_SIZE - buf_bytes; /* check room(how many bytes) available out of 512 bytes */
	count = (bytes <= room ? bytes: room); /* count = whatever is not overflowed */
	count1 = count;
	dest = &buf[buf_bytes];
	src = buffer;
	while (count--) *dest++ = *src++;

	/* check if the buffer will be full after src input */
	buf_bytes += count1; /* add #bytes */
	if (buf_bytes == SECTOR_SIZE) {
		/* write the whole block(512-bytes) to the disk */
		write_block(cur_sector, buf);
		clear_buf();
	}

	/* if count1 is bytes, not room. That is, if there is not any more data to copy, then return */
	if (count1 == bytes) return;
	/* if count1 == room, then calculate #bytes not copied yet 
	 * next, copy it for the next sector output buffer */
	bytes -= count1;
	buf_bytes = bytes;
	dest = buf;	/* buf is a new buffer for next sector */
	while (bytes--) *dest++ = *src++; 
}

flush()
{
	/* if remaing data not copied to output disk then, flush them */
	if (buf_bytes == 0) return; /* no remaining data */
	write_block(cur_sector, buf);
	clear_buf();
}

clear_buf()
{
	/* clear buf(output buffer) for next sector/block */
	register char *p;
	for (p = buf; p < &buf[SECTOR_SIZE]; p++) *p = 0;
	buf_bytes = 0;
	cur_sector++;	
}

read_block(blk, buff)
int blk;
char buff[SECTOR_SIZE];
{
  lseek(image, (long)SECTOR_SIZE * (long) blk, 0);
  if (read(image, buff, SECTOR_SIZE) != SECTOR_SIZE) pexit("block read error", "");
}

write_block(blk, buff)
int blk;
char buff[SECTOR_SIZE];
{
	lseek(image, (long)SECTOR_SIZE * (long) blk, 0);	/* http://www.minix-vmd.org/pub/minix/2.0.0/manuals/CAT2/LSEEK.2 */
	if (write(image, buff, SECTOR_SIZE) != SECTOR_SIZE) pexit("block write error", "");
}

pexit(s1, s2)
char *s1;
char *s2;
{
	printf("Build: %s%s\n", s1, s2);
	exit(1);
}

patch_bootblok(all_size)
long all_size;
{
	/* put ip and cs values of fsck in the last two words of the bookblok
	 * if fsck is sep I&D, we must also provide the ds value (addr. 506)
	 * total sectors to load (addr. 504)
	 */

	long fsck_begin;
	unsigned short ip, cs, ds, ubuf[SECTOR_SIZE/2], sectors;
	
	if (core_size % 16 != 0) 
		pexit("core os size is not multiple of 16 bytes", "");

	fsck_begin = KERNEL_BEGIN + core_size;
	cs = fsck_begin >> CLICK_SHIFT;	/* 20-bits memory to 16-bits memory to make it suitable for register */
	ip = 0;
	if (sizes[num_programs - FSCK_OFFSET].sep_id)
		ds = cs + (sizes[num_programs - FSCK_OFFSET].text_size >> CLICK_SHIFT);
	else
		ds = cs;
	sectors = (unsigned) (all_size / SECTOR_SIZE);	/* for 504 */
	
	/* why SECTOR/2? because registers are 2 bytes */
 	read_block(0, ubuf);	/* read in bootblok */	
	ubuf[(SECTOR_SIZE/2) - 4] = sectors + 1;	/* + 1 because sectors was rounded down by / */
	ubuf[(SECTOR_SIZE/2) - 3] = ds;
	ubuf[(SECTOR_SIZE/2) - 2] = ip;
	ubuf[(SECTOR_SIZE/2) - 1] = cs;
	write_block(0, ubuf);

printf("sectors: %d\nds: %d\nip: %d\ncs: %d\n", sectors+1, ds, ip, cs);
}

patch_kernel()
{
/* This program now has information about the sizes of the kernel, mm, fs, and
 * init.  This information is patched into the kernel as follows. The first 8
 * words of the kernel data space are reserved for a table filled in by build.
 * The first 2 words are for kernel, then 2 words for mm, then 2 for fs, and
 * finally 2 for init.  The first word of each set is the text size in clicks;
 * the second is the data+bss size in clicks.  If separate I & D is NOT in
 * use, the text size is 0, i.e., the whole thing is data.
 *
 * In addition, the DS value the kernel is to use is computed here, and loaded
 * at location 4 in the kernel's text space.  It must go in text space because
 * when the kernel starts up, only CS is correct.  It does not know DS, so it
 * can't load DS from data space, but it can load DS from text space.
 */
	int i;
	unsigned short text, data, bss, text_clicks, data_clicks, ds;	/* short and int are the same size */ 
	long data_offset;

	/* See if the magic number exists at the first entry of kernel data space */
	data_offset = BOOTBLOK_SIZE + (long)sizes[KERN].text_size;	/* starting point of kernel data */	
	i = (get_byte(data_offset+1L) << 8) + get_byte(data_offset);	/* read the first and second byte of the kernel data */
	if (i != KERNEL_D_MAGIC) {
		pexit("kernel data space: no magic # at the first entry", "");	
	}

	for (i = 0; i < num_programs - 1; i++) 	{ 	/* fsck is an exception in this loop */
		text = sizes[i].text_size;
		data = sizes[i].data_size;
		bss = sizes[i].bss_size;
	
		if (sizes[i].sep_id) {
			text_clicks = text >> CLICK_SHIFT;	/* >> CLICK_SHIFT makes it to 20bit-value, which is physical memory value */
			data_clicks = (data + bss) >> CLICK_SHIFT;
		} else {
			text_clicks = 0;
			data_clicks = (text + data + bss) >> CLICK_SHIFT;	
		}
		/* put into the first .word Refer to mpx88.s */
		put_byte(data_offset + 4*i + 0L, (text_clicks>>0) & 0377);
		put_byte(data_offset + 4*i + 1L, (text_clicks>>8) & 0377);
		/* put into the second .word */	
		put_byte(data_offset + 4*i + 2L, (data_clicks>>0) & 0377);
		put_byte(data_offset + 4*i + 3L, (data_clicks>>0) & 0377);
	}

	/* write the DS value into address 0x0060:0004 of the kernel text space */
	if (sizes[KERN].sep_id == 0)
		ds = KERNEL_BEGIN >> CLICK_SHIFT;
	else
		ds = (KERNEL_BEGIN + sizes[KERN].text_size) >> CLICK_SHIFT;
	put_byte(BOOTBLOK_SIZE + DS_OFFSET, ds & 0377);
	put_byte(BOOTBLOK_SIZE + DS_OFFSET + 1L, (ds >> 8) & 0377);

}

main(argc, argv)
int argc;
char *argv[];
{
	int i;

	if (argc < MIN_ARGC)
		pexit("at least 3 arguments are expected. ", "");
	/* create the output file  */	
	create_image(argv[argc - 1]);

	/* copy bootblok to the output file */
	copy_boot(argv[1]);

	num_programs = argc - MIN_ARGC;
	/* copy programs to diskette to build os image */
	for (i = 0; i < num_programs; i++)
		copy_prog(i, argv[i+2]);
	
	flush();
	printf("boot disk created!\n");

	/* Make the three patches to the output file or diskette. */
	patch_bootblok(all_size);
	patch_kernel();
	exit(0);
}
