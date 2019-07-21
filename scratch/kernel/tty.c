#include <stdint.h>
#include <message.h>
#include <std.h>
#include "../h/common.h"
#include "../h/util.h"
#include "tty.h"
#include "common.h"
#include "irq.h"

#define INITIAL_RELEASE_SCAN_CODE	0x80
/* Scan codes to ASCII for unshifted keys */
static char un_shift[] = {
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','[',']',012,0202,'a','s',
 'd','f','g','h','j','k','l',';',      047,0140,0200,0134,'z','x','c','v',
 'b','n','m',',','.','/',0201,'*',     0203,' ',0204,0241,0242,0243,0244,0245,
 0246,0247,0250,0251,0252,0205,0210,0267,  0270,0271,0211,0264,0265,0266,0214
,0261,  0262,0263,'0',0177
};

/* Scan codes to ASCII for shifted keys */
static char shift[] = {
 0,033,'!','@','#','$','%','^',        '&','*','(',')','_','+','\b','\t',
 'Q','W','E','R','T','Y','U','I',      'O','P','{','}',015,0202,'A','S',
 'D','F','G','H','J','K','L',':',      042,'~',0200,'|','Z','X','C','V',
 'B','N','M','<','>','?',0201,'*',    0203,' ',0204,0221,0222,0223,0224,0225,
 0226,0227,0230,0231,0232,0204,0213,'7',  '8','9',0211,'4','5','6',0214,'1',
 '2','3','0',177
};

static struct _tty_struct {
	char buffer[MAX_MESSAGE_CHAR_SIZE];
	int suspended_caller;	// e.g. FS
	int client_pid;	// e.g shell
	int count;	/* # chars in buffer */
} tty_struct;

int caller;
int dont_reply = 0;

// The VGA framebuffer starts at 0xB8000.
uint16_t *video_memory = VIDEO_MEMORY;
// Stores the cursor position.
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

static message_t msg;

// Updates the hardware cursor.
static void move_cursor()
{
	// The screen is 80 characters wide...
	uint16_t cursorLocation = cursor_y * 80 + cursor_x;
	outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
	outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
	outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
	outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll()
{

	// Get a space character with the default colour attributes.
	uint8_t attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

	// Row 25 is the end, this means we need to scroll up
	if(cursor_y >= 25)
	{
		// Move the current text chunk that makes up the screen
		// back in the buffer by a line
		int i;
		for (i = 0*80; i < 24*80; i++)
		{
			video_memory[i] = video_memory[i+80];
		}

		// The last line should now be blank. Do this by writing
		// 80 spaces to it.
		for (i = 24*80; i < 25*80; i++)
		{
			video_memory[i] = blank;
		}
		// The cursor should now be on the last line.
		cursor_y = 24;
	}
}

void out_char(char c)
{
	// The background colour is black (0), the foreground is white (15).
	uint8_t backColour = 0;
	uint8_t foreColour = 15;

	// The attribute byte is made up of two nibbles - the lower being the 
	// foreground colour, and the upper the background colour.
	uint8_t  attributeByte = (backColour << 4) | (foreColour & 0x0F);
	// The attribute byte is the top 8 bits of the word we have to send to the
	// VGA board.
	uint16_t attribute = attributeByte << 8;
	uint16_t *location;

	// Handle a backspace, by moving the cursor back one space
	if (c == 0x08 && cursor_x)
	{
		cursor_x--;
	}

	// Handle a tab by increasing the cursor's X, but only to a point
	// where it is divisible by 8.
	else if (c == 0x09)
	{
		cursor_x = (cursor_x+8) & ~(8-1);
	}

	// Handle carriage return
	else if (c == '\r')
	{
		cursor_x = 0;
	}

	// Handle newline by moving cursor back to left and increasing the row
	else if (c == '\n')
	{
		cursor_x = 0;
		cursor_y++;
	}
	// Handle any other printable character.
	else if(c >= ' ')
	{
		location = video_memory + (cursor_y*80 + cursor_x);
		*location = c | attribute;
		cursor_x++;
	}

	// Check if we need to insert a new line because we have reached the end
	// of the screen.
	if (cursor_x >= 80)
	{
		cursor_x = 0;
		cursor_y ++;
	}

	// Scroll the screen if needed.
	scroll();
	// Move the hardware cursor.
	move_cursor();
}

static void echo(char ch)
{
	out_char(ch);
	//flush();
}

static char make_break(char scan_code)
{
	return un_shift[scan_code];
		
}

static int do_char(char scan_code)
{
	char ch;
	ch = make_break(scan_code);	// convert scan code to ASCII code
	if (ch == '\r')	// if a key is carrige return(enter key), also apply new line feed
		echo('\n');
	else
		echo(ch);
	tty_struct.count--;

	return ch;
}

static int do_chars()
{
	int rv;
	if (msg.ucharacter[0] < INITIAL_RELEASE_SCAN_CODE)
	{
debug_printf("key pressed: %x\n", msg.ucharacter[0]);
		rv = do_char(msg.ucharacter[0]);
		if (tty_struct.suspended_caller >= 0)	// there is suspended caller due to no input before
		{
			caller = tty_struct.suspended_caller;
			msg.integer[PID_IDX_IN_MESSAGE] = tty_struct.client_pid;
			msg.integer[RETURN_IDX_IN_MESSAGE] = rv;
			//msg.type = REVIVE_REPLY;

			tty_struct.suspended_caller = -1;	// there is no more pid to wait for input. So, reset tty_struct
			dont_reply = 0;
		}
		else 
			dont_reply = 1;

		return rv;
	}
	else
	{
debug_printf("key released: %x\n", msg.ucharacter[0]);
		//tty_struct.count--;
		dont_reply = 1;

		return 0;
	}

	
}

static int tty_read()
{
	tty_struct.suspended_caller = msg.src_pid;	// for now, we assume that there is no input yet at this momenet of running this function, so suspend the caller
	tty_struct.client_pid = msg.integer[PID_IDX_IN_MESSAGE];
	dont_reply = 1;	// it doesn't reply to caller and continue

	return 0;	
	/*	
	tty_struct.caller = msg.src_pid;
	tty_struct.client_pid = msg.integer[PID_IDX_IN_MESSAGE];

	if (tty_struct.count <= 0)
		return SUSPEND_REPLY;
	*/
}

static int tty_write()
{
	int size = msg.integer[SIZE_IDX_IN_MESSAGE];
	int msg_buf_address = msg.uinteger[BUF_ADDRESS_IDX_IN_MESSAGE];
	uint32_t msg_buffer_phy_address = vir_to_phy_address(msg_buf_address, caller);

	do 
	{	
		echo(*((char *)msg_buffer_phy_address++));	// single char
	} while (--size);

	dont_reply = 0;
debug_printf("tty_write: echo finished\n");
	return 0;	
}

int tty_task()
{
	int opcode, status;
	//init_tty();
	while (1)
	{ 
		receive(ANY_PROC, &msg); 
		opcode = msg.type;
		caller = msg.src_pid;
		switch (opcode)
		{
			case TTY_CHAR_INT:
				status = do_chars();	
				break;

			case TTY_READ:
				status = tty_read();			
				break;

			case TTY_WRITE:
				status = tty_write();			
				break;
		}
		if (dont_reply)
			continue;
		msg.integer[RETURN_IDX_IN_MESSAGE] = status;
		send(caller, &msg);
	}

	return 0;
}

/* when keyboard interrupt occurs, this function massage it before sending the message to tty task
 */
message_t keyboard()
{
	message_t msg;
	char scan_code, val;
	
	scan_code = inb(PORT_A_8255);	// read scan code from the data port
	/* keyboard will not send next scan code until previous one "acknowledged"
	 * to acknowledge, bit 7 (0->1, then 1->0)
	 */
	val = inb(PORT_B_8255);
	outb(PORT_B_8255, val | SCAN_CODE_ACK_BIT_1);
	outb(PORT_B_8255, val & SCAN_CODE_ACK_BIT_0); 

	tty_struct.buffer[0] = scan_code;
debug_printf("keyboard() %x, %x\n", scan_code, proc_table[2].flags);
	//tty_struct.count++;
	msg.ucharacter[0] = tty_struct.buffer[0];
	return msg;
}
