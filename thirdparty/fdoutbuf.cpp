#include "fdoutbuf.h"
#include <dlfcn.h>
#include <streambuf>
#include <iostream>

fdostream sie::cout;
const char *libdlfcnname = "./../libary/libdlfcn.so";
typedef ssize_t (*PFN_Writeex) (int __fd, const void *__buf, size_t __n);
class fdoutbufprivate
{
public:
    fdoutbufprivate():
        writeex(nullptr)
    {
        handle = dlopen (libdlfcnname, RTLD_GLOBAL | RTLD_LAZY | RTLD_DEEPBIND);
        if (!handle) {
            std::cout << "load libdlfcn.so faild." <<
                      std::endl;
            return;
        }
        writeex = (ssize_t(*)(int __fd, const void *__buf, size_t __n))dlsym(RTLD_NEXT, "writeex");
    }
    ssize_t write(int __fd, const void *__buf, size_t __n)
    {
        return writeex(__fd, __buf, __n);
    }
private:
    void *handle;
    PFN_Writeex writeex;
};

fdoutbuf::fdoutbuf(int _fd): fd(_fd),
    d_ptr(new fdoutbufprivate)
{

}

std::streambuf::int_type fdoutbuf::overflow(std::streambuf::int_type c)
{
    if (c != EOF) {
        char z = c;
        if (d_ptr->write(fd, &z, 1) != 1) {
            return EOF;
        }
    }
    return c;
}

std::streamsize fdoutbuf::xsputn(const char *s, std::streamsize num)
{
    return d_ptr->write(fd, s, num);
}

