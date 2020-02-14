/* fsck - file system checker */

/* why need all the implementation of basic functions?
 * why not use libc libraries?
 * it's because libc libries are related to kernel.
 * for example, printf()
 * However, at this point, there is no kernel 
 */

/*#include "../h/const.h"
#include "../h/type.h"
#include "../fs/const.h"
#include "../fs/type.h"*/

#define MAXWIDTH         32

#define between(c,l,u)  ((unsigned short) ((c) - (l)) <= ((u) - (l)))
#define isprint(c)      between(c, ' ', '~')
#define isdigit(c)      between(c, '0', '9')
#define islower(c)      between(c, 'a', 'z')
#define isupper(c)      between(c, 'A', 'Z')
#define toupper(c)      ( (c) + 'A' - 'a' )

#define quote(x)        x
#define nextarg(t)      (*argp.quote(u_)t++)

#define prn(t,b,s)      { printnum((long)nextarg(t),b,s,width,pad); width = 0; }
#define prc(c)          { width -= printchar(c, mode); }

union types {
        int      *u_char;       /* %c */
        int      *u_int;        /* %d */
        unsigned *u_unsigned;   /* %u */
        long     *u_long;       /* %D */
        char    **u_charp;      /* %s */
};

char *prog; 	/* program name (fsck) */
/*char *rwbuf;i*/	/* one block buffer cache */
/*char rwbuf1[BLOCK_SIZE];*/	/* in case of a DMA-overrun under DOS ... */
int cylsiz, tracksiz;
int virgin = 1;
int sectors_trk = 9;	/* #sectors per track for 360K */

printf(fmt, args)
char *fmt;
{
        doprnt(fmt, &args);
}

/* Print the arguments pointer to by `arg' according to format.
 */
doprnt(format, argp)
char *format;
union types argp;
{
        register char *fmt, *s;
        register short width, pad, mode;

        for (fmt = format; *fmt != 0; fmt++)
                switch(*fmt) {
                case '\n': putchar('\r');
                default:   putchar(*fmt);
                           break;
                case '%':
                        if (*++fmt == '-')
                                fmt++;
                        pad = *fmt == '0' ? '0' : ' ';
                        width = 0;
                        while (isdigit(*fmt)) {
                                width *= 10;
                                width += *fmt++ - '0';
                        }
                        if (*fmt == 'l' && islower(*++fmt))
                                *fmt = toupper(*fmt);
                        mode = isupper(*fmt);
                        switch (*fmt) {
                        case 'c':
                        case 'C':  prc(nextarg(char));          break;
                        case 'b':  prn(unsigned,  2, 0);        break;
                        case 'B':  prn(long,      2, 0);        break;
                        case 'o':  prn(unsigned,  8, 0);        break;
                        case 'O':  prn(long,      8, 0);        break;
                        case 'd':  prn(int,      10, 1);        break;
                        case 'D':  prn(long,     10, 1);        break;
                        case 'u':  prn(unsigned, 10, 0);        break;
                        case 'U':  prn(long,     10, 0);        break;
                        case 'x':  prn(unsigned, 16, 0);        break;
                        case 'X':  prn(long,     16, 0);        break;
                        case 's':
                        case 'S':  s = nextarg(charp);
                                   while (*s) prc(*s++);        break;
                        case '\0': break;
                        default:   putchar(*fmt);
                        }
                        while (width-- > 0)
                                putchar(pad);
                }
}

/* Print the number n.
 */
printnum(n, base, sign, width, pad)
long n;
int base, sign;
int width, pad;
{
        register short i, mod;
        char a[MAXWIDTH];
        register char *p = a;

        if (sign)
                if (n < 0) {
                        n = -n;
                        width--;
                }
                else
                        sign = 0;
        do {            /* mod = n % base; n /= base */
                mod = 0;
                for (i = 0; i < 32; i++) {
                        mod <<= 1;
                        if (n < 0)
                                mod++;
                        n <<= 1;
                        if (mod >= base) {
                                mod -= base;
                                n++;
                        }
                }
                *p++ = "0123456789ABCDEF"[mod];
                width--;
        } while (n);
        while (width-- > 0)
                putchar(pad);
        if (sign)
                *p++ = '-';
        while (p > a)
                putchar(*--p);
}

/* Print the given character. */
putchar(c){
        if (c == '\n')
                putc('\r');
        putc(c);
}

/* Print the character c.
 */
printchar(c, mode){
        if (mode == 0 || (isprint(c) && c != '\\')) {
                putchar(c);
                return(1);
        }
        else {
                putchar('\\');
                switch (c) {
                case '\0':      putchar('0'); break;
                case '\b':      putchar('b'); break;
                case '\n':      putchar('n'); break;
                case '\r':      putchar('r'); break;
                case '\t':      putchar('t'); break;
                case '\f':      putchar('f'); break;
                case '\\':      putchar('\\'); break;
                default:        printnum((long) (c & 0xFF), 8, 0, 3, '0');
                                return(4);
                }
                return(2);
        }
}

main(argc, argv)
int argc;
char **argv;
{
	register char **clist = 0, **ilist = 0, **zlist = 0;
	register c, command;

	if (virgin)
		sectors_trk = tracksiz;
	virgin = 0;
	if (tracksiz < 9 || cylsiz < 18)
		printf("Bootblok gave bad tracksiz\n"); 

	/*rwbuf = rwbuf1;*/	/* TOKNOW: why is this needed? */
	prog = "fsck";
	printf("\n\n\n\n");
	while (1) {
		printf("\nHit key as follows:\n\n");
                printf("    =  start MINIX (root file system in drive 0)\n");
                /* TODO: implement followings as you want
		 *printf("    f  check the file system (first insert any file system diskette)\n");
                 *printf("    l  check and list file system (first insert any file system diskette)\n");
                 *printf("    m  make an (empty) file system (first insert blank, formatted diskette)\n");
                 *printf("    h  check hard disk file system\n"); 
		 */
                printf("\n# ");
		c = getc(); 
		command = c & 0xFF; /* command is same thing as c */
		printf("%c\n", command);

		switch (command) {
			case '=':
				return ((c >> 8) & 0xFF);	/* return 1 byte type */
			default: 
				printf("Illegal command\n");
		}
		
	}
}