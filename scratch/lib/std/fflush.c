#include <std.h>

void fflush(FILE *stream)
{
        int count;

        if ( testflag(stream,UNBUFF) || !testflag(stream,WRITEMODE) )
                return 0;

        if ( stream->_count <= 0 )
                return 0;

        count = write(stream->_fd,stream->_buf,stream->_count);

        if ( count == stream->_count) {
                stream->_count = 0;
                stream->_ptr   = stream->_buf;
                return count;
        }

        stream->_flags |= _ERR;
        return EOF;
}

