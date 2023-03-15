#include "objectfactory.h"
#include <iostream>
#include "clr2.h"
#include "fdoutbuf.h"
#include "functioninvoker.h"
#include <assert.h>
#include "clelapsedtimer.h"
#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

class CLImage2dFun : public FunctionInvoker
{
public:
    std::string cmdName() override
    {
        return "cl2d";
    }
    std::string describe() override
    {
        return " calc with opencl image 2d";
    }
    bool process(unsigned char *srcbuf, unsigned char *dstbuf, int width, int height,
                 int channels) override
    {
        assert(srcbuf);

        const unsigned int imgsize = width * height * channels;

        // unsigned char *img = new unsigned char[imgsize];

        cl_int    status;
        // ------------------setup-----------------------//
        //1 获取CL 平台信息
        cl_uint num_platforms;
        cl_platform_id *platforms  = CLR2::get_cl_Platforms(num_platforms);
        if (platforms == nullptr) {
            std::cout << " get platforms failed." << std::endl;
            return false;
        }
        // 2  获取 平台下的设备信息
        cl_uint num_devices;
        cl_device_id *devices = CLR2::get_cl_device_id(platforms[0], num_devices);
        if (devices == nullptr) {
            std::cout << " get device id failed." << std::endl;
            return false;
        }

        // 3 为设备通信创建上下文
        cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

        // 4 为设备创建通信的消息队列
        cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

        if (commandQueue == 0) {
            std::cout << "Can't create command queue\n";
            clReleaseContext(context);
            return false;
        }


        cl_image_format image_format;
        image_format.image_channel_order = CL_RGBA;
        image_format.image_channel_data_type = CL_UNSIGNED_INT8;

        cl_mem img_input = clCreateImage2D(context,
                                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           &image_format,
                                           width,
                                           height,
                                           0,
                                           srcbuf,
                                           &status);
        cl_mem img_out = clCreateImage2D(context,
                                         CL_MEM_WRITE_ONLY,
                                         &image_format,
                                         width,
                                         height,
                                         0,
                                         NULL,
                                         &status);

        cl_program program = CLR2::load_program(context, "img2dmirror.cl");
        if (program == 0) {
            sie::cout << "load program failed." << std::endl;
        }
        CLElapsedTimer timer;
        timer.start();
        cl_kernel kernel = clCreateKernel(program, "mirrorimg2d", NULL);
        if (kernel == 0) {
            sie::cout << "Can't load kernel\n";
            clReleaseProgram(program);

            clReleaseMemObject(img_input);

            clReleaseMemObject(img_out);

            clReleaseCommandQueue(commandQueue);

            clReleaseContext(context);
            return false;
        }

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &img_input);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &img_out);

        size_t global[3] = {(size_t)width,  (size_t)height, 0};
        status = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);

        size_t origin[] = {0, 0, 0};
        size_t region[] = {(size_t)width, (size_t)height, 1};

        cl_int err = clEnqueueReadImage(commandQueue, img_out, CL_TRUE, origin,
                                        region, 0, 0, dstbuf, 0, NULL, NULL);
        sie::cout << "  用时(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) << std::endl;
        //release device
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(img_input);
        clReleaseMemObject(img_out);
        clReleaseContext(context);

        if (devices != NULL) {
            free(devices);
            devices = NULL;
        }
        return true;
    }
};

REGISTER_OBJECT(FunctionInvoker, CLImage2dFun)
