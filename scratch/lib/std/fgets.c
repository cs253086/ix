#include <std.h>

char *fgets(char *s, int size, FILE *stream)
{
	int ch;
	char *ptr;

	ptr = s;
	while ( --size > 0 && (ch = getc(stream)) != EOF){
		*ptr++ = ch;
		if ( ch == '\n')
			break;
	}
	if (ch == EOF && ptr == s)
		return NULL;
	*ptr = '\0';

	return s;
}

