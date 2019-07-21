#include <stdint.h>
#include "../h/message.h"
#include "../h/syscall.h"
#include "../h/util.h"
#include "tty.h"
#include "common.h"
#include "irq.h"

#define INITIAL_RELEASE_SCAN_CODE	0x80
/* Scan codes to ASCII for unshifted keys */
static char un_shift[] = {
 0,033,'1','2','3','4','5','6',        '7','8','9','0','-','=','\b','\t',
 'q','w','e','r','t','y','u','i',      'o','p','[',']',015,0202,'a','s',
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

static char tty_char_buffer[TTY_MAX_CHAR_CAPACITY];
static message_t msg;

void out_char(char ch)
{
	primitive_putc(ch);
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

static void do_char(char scan_code)
{
	char ch;
	ch = make_break(scan_code);
	if (ch == '\r')	// if a key is carrige return(enter key), also apply new line feed
		echo('\n');
	else
		echo(ch);
}

static void do_chars(message_t *msg)
{
	if (((unsigned char *)(msg->buf))[0] < INITIAL_RELEASE_SCAN_CODE)
	{
debug_printf("key pressed: %x\n", ((unsigned char *)(msg->buf))[0]);
		do_char(((unsigned char *)(msg->buf))[0]);
	}
	else
	{
debug_printf("key released: %x\n", ((unsigned char *)(msg->buf))[0]);
	}
}

int tty_task()
{
	int opcode;
	//init_tty();
	while (1)
	{ 
		receive(ANY_PROC, &msg); 
		opcode = msg.type;
		switch (opcode)
		{
			case TTY_CHAR_INT:
				do_chars(&msg);	
				break;
		}
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

	
	tty_char_buffer[0] = scan_code;
	msg.buf = (void *)tty_char_buffer;
	return msg;
}
