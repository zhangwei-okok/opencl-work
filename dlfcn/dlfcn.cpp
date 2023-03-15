#ifdef _MSC_VER
    #include<io.h>
#else
    #include<unistd.h>
#endif

extern "C"  __attribute__((visibility("default")))
ssize_t writeex (int __fd, const void *__buf, size_t __n)
{
    return write(__fd, __buf, __n);
}
