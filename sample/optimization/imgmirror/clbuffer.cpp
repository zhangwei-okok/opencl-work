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

class CLBufferFun : public FunctionInvoker
{
public:
    std::string cmdName() override
    {
        return "cl";
    }
    std::string describe() override
    {
        return " calc with opencl buffer.";
    }
    bool process(unsigned char *srcbuf, unsigned char *dstbuf, int width, int height,
                 int channels) override
    {
        assert(srcbuf);
        assert(dstbuf);
        const unsigned int imgsize = width * height * channels;

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
        cl_mem input_buffer, output_buffer, imgwidth, imgheight, imgchannels;
        input_buffer = clCreateBuffer(
                           context,
                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           imgsize,
                           srcbuf,
                           &status);

        output_buffer = clCreateBuffer(
                            context,
                            CL_MEM_WRITE_ONLY,
                            imgsize,
                            NULL,
                            &status);

        imgwidth    = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(int), &width, &status);
        imgheight   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(int), &height, &status);
        imgchannels = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(int), &channels, &status);

        if (input_buffer == 0 || output_buffer == 0) {
            sie::cout << "Can't create OpenCL image buffer\n";
            clReleaseMemObject(input_buffer);
            clReleaseMemObject(output_buffer);
            clReleaseMemObject(imgwidth);
            clReleaseMemObject(imgheight);
            clReleaseMemObject(imgchannels);
            clReleaseContext(context);
            return false;
        }

        cl_program program = CLR2::load_program(context, "imgmirror.cl");
        if (program == 0) {
            sie::cout << "load program failed." << std::endl;
        }
        CLElapsedTimer timer;
        timer.start();

        cl_kernel kernel = clCreateKernel(program, "mirror", NULL);
        if (kernel == 0) {
            sie::cout << "Can't load kernel\n";
            clReleaseProgram(program);

            clReleaseMemObject(input_buffer);

            clReleaseMemObject(output_buffer);

            clReleaseCommandQueue(commandQueue);

            clReleaseContext(context);
            return false;
        }

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
        clSetKernelArg(kernel, 2, sizeof(cl_mem), &imgwidth);
        clSetKernelArg(kernel, 3, sizeof(cl_mem), &imgheight);
        clSetKernelArg(kernel, 4, sizeof(cl_mem), &imgchannels);

        size_t localThreads[2]  = {32, 4};   //工作组中工作项的排布
        size_t globalThreads[2] = {((width  + localThreads[0] - 1) / localThreads[0]) *localThreads[0],
                                   ((height + localThreads[1] - 1) / localThreads[1]) *localThreads[1]
                                  };   //整体排布


        std::cout << " globalThreads: "  << globalThreads[0] << " " << globalThreads[1] << std::endl;
        cl_event evt;
        status = clEnqueueNDRangeKernel(commandQueue, kernel,  //启动内核
                                        2, 0, globalThreads, localThreads,
                                        0, NULL, &evt);  //内核执行完成后，会将evt置为CL_SUCCESS/CL_COMPLETE
        clWaitForEvents(1, &evt);   //等待命令事件发生
        clReleaseEvent(evt);



        status = clEnqueueReadBuffer(commandQueue, output_buffer,
                                     CL_TRUE,
                                     0,
                                     imgsize,
                                     dstbuf, 0, NULL, NULL);
        sie::cout << "  用时(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) << std::endl;
        //release device
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_buffer);
        clReleaseMemObject(imgwidth);
        clReleaseMemObject(imgheight);
        clReleaseMemObject(imgchannels);
        clReleaseContext(context);

        if (devices != NULL) {
            free(devices);
            devices = NULL;
        }
        return true;
    }
};

REGISTER_OBJECT(FunctionInvoker, CLBufferFun)
