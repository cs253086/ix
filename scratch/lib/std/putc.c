#include <std.h>

int putc(char ch, FILE *stream)
{
	int n, didwrite = 0;

	if (testflag(stream, (_ERR | _EOF)))
		return EOF; 

	if ( !testflag(stream, WRITEMODE))
		return EOF;

	if ( testflag(stream, UNBUFF)){
		n = write(stream->_fd, &ch, 1);
		stream->_count = 1;
		didwrite++;
	}
	else{
		//n = write(stream->_fd, &ch, 1);
		*stream->_ptr++ = ch;
		if ((++stream->_count) >= STD_IN_OUT_BUF_SIZE && !testflag(stream, STRINGS) ){
			n = write(stream->_fd,stream->_buf, stream->_count);
			stream->_ptr = stream->_buf;
			//didwrite++;
		}
	}
#if 0
	if (didwrite){
		if (n<=0 || stream->_count != n){
			if (n < 0)
				stream->_flags |= _ERR;
			else
				stream->_flags |= _EOF;
			return EOF;
		}
		stream->_count=0;
	}
#endif
	return ch;
}

