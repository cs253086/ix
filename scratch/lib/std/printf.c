#include <stdarg.h>
#include <stdint.h>
#include <std.h>
#include "../../h/common.h"
#include "../../h/util.h"

#define MAXDIGITS	12

#if 0
// Updates the hardware cursor.
static void move_cursor()
{
    // The screen is 80 characters wide...
    uint16_t cursorLocation = cursor_y * 80 + cursor_x;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.  outb(0x3D5, cursorLocation);      // Send the low cursor byte.
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
#endif

static int bintoascii(long num, int radix, char a[MAXDIGITS])
{

  int i, n, hit, negative;

  negative = 0;
  if (num == 0) {a[0] = '0'; return(1);}
  if (num < 0 && radix == 10) {num = -num; negative++;}
  for (n = 0; n < MAXDIGITS; n++) a[n] = 0;
  n = 0;

  do {
        if (radix == 10) {a[n] = num % 10; num = (num -a[n])/10;}
        if (radix ==  8) {a[n] = num & 0x7;  num = (num >> 3) & 0x1FFFFFFF;}
        if (radix == 16) {a[n] = num & 0xF;  num = (num >> 4) & 0x0FFFFFFF;}
        n++;
  } while (num != 0);

  /* Convert to ASCII. */
  hit = 0;
  for (i = n - 1; i >= 0; i--) {
        if (a[i] == 0 && hit == 0) {
                a[i] = ' ';
        } else {
                if (a[i] < 10)
                        a[i] += '0';
                else
                        a[i] += 'A' - 10;
                hit++;
        }
  }
  if (negative) a[n++] = '-';
  return(n);
}

#if 0
// Writes a single character out to the screen.
void putc(char c)
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

// Clears the screen, by copying lots of spaces to the framebuffer.
void clear_screen()
{
    // Make an attribute byte for the default colours
    uint8_t attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
    uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

    int i;
    for (i = 0; i < 80*25; i++)
    {
        video_memory[i] = blank;
    }

    // Move the hardware cursor back to the start.
    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}
#endif

int printf(const char *s, ...)
{
	int w, k, r;
	unsigned int u;
	long l, *lp;
	char a[MAXDIGITS], *p, *p1, c;
	va_list ap;

	va_start(ap, s);
	while (*s != '\0') {
	if (*s !=  '%') {
		putc(*s++, stdout);
		continue;
	}

	w = 0;
	s++;
	while (*s >= '0' && *s <= '9') {
		w = 10 * w + (*s - '0');
		s++;
	}

	switch(*s) {
	    case 'd':   k = va_arg(ap, int); l = k;  r = 10;  break;
	    case 'o':   k = va_arg(ap, int); u = k; l = u;  r = 8;  break;
	    case 'x':   k = va_arg(ap, int); u = k; l = u;  r = 16;  break;
	    case 'D':   l = va_arg(ap, long);  r = 10; break;
	    case 'O':   l = va_arg(ap, long);  r = 8; break;
	    case 'X':   l = va_arg(ap, long);  r = 16; break;
	    case 'c':   k = va_arg(ap, int); putc((char) k, stdout); s++; continue;
	    case 's':   p = va_arg(ap, char *); 
		p1 = p;
		while(c = *p++) putc(c, stdout); s++;
		if ( (k = w - (p-p1-1)) > 0) while (k--) putc(' ', stdout);
		continue;
	    default:    putc('%', stdout); putc(*s++, stdout); continue;
	}

	k = bintoascii(l, r, a);
	if ( (r = w - k) > 0) while(r--) putc(' ', stdout);
	for (r = k - 1; r >= 0; r--) putc(a[r], stdout);
	s++;
	}
	va_end(ap);
	fflush(stdout);
}
