char *strcpy(char *dest, const char *src)
{
	while (*src != '\0')
		*dest++ = *src++;
	*dest = '\0';

	return dest;
}
