void copy(void *dest, void *src, int bytes)
{
	/* copy character-by-character. */
	int n = bytes;
	if (n > 0)
	{
		char *dp = (char *) dest;
		char *sp = (char *) src;

		do { *dp++ = *sp++; } while (--n);
	}
}
