#ifndef FUNCTION_INVOKER_H
#define FUNCTION_INVOKER_H
#include <iostream>

class FunctionInvoker
{
public:
    FunctionInvoker()
    {

    }
    virtual ~FunctionInvoker()
    {

    }
    virtual std::string cmdName() = 0;
    virtual std::string describe() = 0;
public:
    virtual bool process(unsigned char *srcbuf, unsigned char *dstbuf, int width, int height,
                         int channels) = 0;
};

#endif
