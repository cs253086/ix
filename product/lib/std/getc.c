#include <std.h>

//http://forum.osdev.org/viewtopic.php?f=15&t=27192
int getc(FILE *stream)
{
        char ch;

        if ( testflag(stream, (_EOF | _ERR )))
                return EOF;
	
        if ( !testflag(stream, READMODE) )
                return EOF;

        if (--stream->_count <= 0){

                if ( testflag(stream, UNBUFF) )
                        stream->_count = read(stream->_fd, &ch, 1);
                else
                        //stream->_count = read(stream->_fd, stream->_buf, STD_IN_OUT_BUF_SIZE);
                        stream->_count = read(stream->_fd, stream->_buf, 1);

                if (stream->_count <= 0){
                        if (stream->_count == 0)
                                stream->_flags |= _EOF;
                        else
                                stream->_flags |= _ERR;

                        return EOF;
                }
                else
                        stream->_ptr = stream->_buf;
        }

        if (testflag(stream,UNBUFF))
                return (ch & CMASK);
        else
                return (*stream->_ptr++ & CMASK);
}

