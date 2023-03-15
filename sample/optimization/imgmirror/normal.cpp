#include "objectfactory.h"
#include <iostream>
#include "clr2.h"
#include "ig.h"
#include "fdoutbuf.h"
#include "functioninvoker.h"
#include <assert.h>
#include "clelapsedtimer.h"
#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

class CLNormal : public FunctionInvoker
{
public:
    std::string cmdName() override
    {
        return "normal";
    }
    std::string describe() override
    {
        return " calc with cpu.";
    }

    bool process(unsigned char *srcbuf, unsigned char *dstbuf, int width, int height,
                 int channels) override
    {
        assert(srcbuf);
        assert(dstbuf);
        const unsigned int imgsize = width * height * channels;
        CLElapsedTimer timer;
        timer.start();
        Ig::levelmirror(srcbuf, dstbuf, width, height, channels);
        sie::cout << "  用时(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) << std::endl;
        return true;
    }
};

REGISTER_OBJECT(FunctionInvoker, CLNormal)
