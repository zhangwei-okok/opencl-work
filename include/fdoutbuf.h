#ifndef H_FDOUTBUF_H
#define H_FDOUTBUF_H
#include <iostream>

class fdoutbufprivate;
class fdoutbuf: public std::streambuf
{
public:
    fdoutbuf(int _fd);
protected:
    virtual int_type overflow(int_type c);
    virtual std::streamsize xsputn(const char *s, std::streamsize num);
protected:
    int fd;
private:
    fdoutbufprivate* d_ptr;
};

class fdostream: public std::ostream
{
protected:
    fdoutbuf buf;
public:
    fdostream(int fd = 1): std::ostream(0), buf(fd)
    {
        rdbuf(&buf);
    }
};

namespace sie
{
 extern fdostream cout;
};

#endif
