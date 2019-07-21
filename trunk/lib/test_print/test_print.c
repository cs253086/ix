#include <stdint.h>
#include <stdarg.h>
#include "../../h/util.h"
#include "../../h/test_print.h"

#define MAXDIGITS	12

static void test_putc(char c)
{
	outb(0xe9, c);
}

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

int test_printf(const char *s, ...)
{
	int w, k, r;
	unsigned int u;
	long l, *lp;
	char a[MAXDIGITS], *p, *p1, c;
	va_list ap;

	va_start(ap, s);
	while (*s != '\0') {
	if (*s !=  '%') {
		test_putc(*s++);
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
	    case 'c':   k = va_arg(ap, int); test_putc((char) k); s++; continue;
	    case 's':   p = va_arg(ap, char *); 
		p1 = p;
		while(c = *p++) test_putc(c); s++;
		if ( (k = w - (p-p1-1)) > 0) while (k--) test_putc(' ');
		continue;
	    default:    test_putc('%'); test_putc(*s++); continue;
	}

	k = bintoascii(l, r, a);
	if ( (r = w - k) > 0) while(r--) test_putc(' ');
	for (r = k - 1; r >= 0; r--) test_putc(a[r]);
	s++;
	}
	va_end(ap);
}

void test_print_pass(const char *func)
{
	test_printf("%s: PASS\n", func);
}

void test_print_fail(int exp, int actual,const char *func)
{
	test_printf("%s: FAIL (exp=%x, actual=%x)\n", func, exp, actual);
} 
