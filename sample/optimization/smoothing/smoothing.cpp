#include <iostream>
#include "clr2.h"
#include "ig.h"
#include "fdoutbuf.h"
#include "clelapsedtimer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define SIZE(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

using namespace std;
#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif


bool dealnormal(unsigned char *srcbuf, int width, int height,
                int channels)
{
    unsigned char *imggray = new unsigned char[width * height];
    CLElapsedTimer timer;
    timer.start();
    Ig::rgb2Gray(srcbuf, imggray, width, height, channels);
    sie::cout << " CPU RGB转灰度用时(ms):" << timer.nsecsElapsed() /
              (1000.0 * 1000.0) << std::endl;
    int ret = stbi_write_bmp("package_gray.bmp", width, height, 1, imggray);
    if (!ret) {
        std::cout << "write image:"  << " failed."
                  << std::endl;
    }
    unsigned char *imggus = new unsigned char[width * height];
    //高斯平滑
    float kernel[3][3] = {
        {1 / 16.0, 2 / 16.0, 1 / 16.0},
        {2 / 16.0, 4 / 16.0, 2 / 16.0},
        {1 / 16.0, 2 / 16.0, 1 / 16.0}
    };

    timer.restart();
    Ig::filter2D(imggray, imggus, height, width, 1, kernel);
    sie::cout << " CPU 高斯平滑用时(ms):" << timer.nsecsElapsed() /
              (1000.0 * 1000.0) << std::endl;

    ret = stbi_write_bmp("package_gray_guss.bmp", width, height, 1, imggus);
    if (!ret) {
        std::cout << "write image:"  << " failed."
                  << std::endl;
    }
    delete []imggus;
    delete []imggray;
    return true;
}

bool dealcl(unsigned char *srcbuf, int width, int height, int channels)
{
    assert(srcbuf);
    CLElapsedTimer timer;
    timer.start();
    cl_int  status;
    cl_mem input_buffer;
    cl_mem output_graybuff;
    cl_mem output_grayguss;
    cl_mem imgwidth;
    cl_mem imgheight;
    cl_mem filter_in;

    const unsigned int imgsize = width * height * channels;
    float filter_kernel[3][3] = {
        {1 / 16.0, 2 / 16.0, 1 / 16.0},
        {2 / 16.0, 4 / 16.0, 2 / 16.0},
        {1 / 16.0, 2 / 16.0, 1 / 16.0}
    };

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

    input_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, imgsize, NULL, &status);

    output_graybuff = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     width * height,
                                     NULL,
                                     &status);
    output_grayguss = clCreateBuffer( context, CL_MEM_WRITE_ONLY,
                                      width * height,
                                      NULL,
                                      &status);


    imgwidth    = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(int), &width, &status);
    imgheight   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(int), &height, &status);

    //创建卷积Kernel
    filter_in = clCreateBuffer(context, CL_MEM_READ_WRITE, 3 * 3 * sizeof(float), NULL, &status);



    cl_program program = CLR2::load_program(context, "smooth.cl", 1);
    if (program == 0) {
        sie::cout << "load program failed." << std::endl;
        return 0;
    }
    timer.restart();
    //1  rgb 转灰度
    cl_kernel kernelrgb2gray = clCreateKernel(program, "kernel_rgb2gray", NULL);
    if (kernelrgb2gray == 0) {
        sie::cout << "Can't load kernel\n";
        clReleaseProgram(program);
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_graybuff);
        clReleaseCommandQueue(commandQueue);
        clReleaseContext(context);
        return false;
    }
    clEnqueueWriteBuffer(commandQueue, input_buffer, CL_TRUE, 0, imgsize, srcbuf, 0,
                         NULL,
                         NULL);

    clSetKernelArg(kernelrgb2gray, 0, sizeof(cl_mem), &input_buffer);
    clSetKernelArg(kernelrgb2gray, 1, sizeof(cl_mem), &output_graybuff);
    clSetKernelArg(kernelrgb2gray, 2, sizeof(cl_mem), &imgheight);
    clSetKernelArg(kernelrgb2gray, 3, sizeof(cl_mem), &imgwidth);

    size_t localThreads[2]  = {32, 4};   //工作组中工作项的排布
    size_t globalThreads[2] = {((width  + localThreads[0] - 1) / localThreads[0]) *localThreads[0],
                               ((height + localThreads[1] - 1) / localThreads[1]) *localThreads[1]
                              };   //整体排布

    cl_event evt;
    status = clEnqueueNDRangeKernel(commandQueue, kernelrgb2gray,  //启动内核
                                    2, 0, globalThreads, localThreads,
                                    0, NULL, &evt);  //内核执行完成后，会将evt置为CL_SUCCESS/CL_COMPLETE
    clWaitForEvents(1, &evt);   //等待命令事件发生
    clReleaseEvent(evt);

    unsigned char *img = new unsigned char[width * height];
    status = clEnqueueReadBuffer(commandQueue, output_graybuff,
                                 CL_TRUE,
                                 0,
                                 width * height,
                                 img, 0, NULL, NULL);

    sie::cout << " OPenCL RGB转灰度用时(ms):" << timer.nsecsElapsed() /
              (1000.0 * 1000.0) << std::endl;

    int ret = stbi_write_bmp("package_gray_cl.bmp", width, height, 1, img);

    if (!ret) {
        std::cout << "write image:"  << " failed."
                  << std::endl;
    }
    timer.restart();
    // 高斯平滑
    cl_kernel kernel_guss = clCreateKernel(program, "kernel_guss", NULL);
    if (kernel_guss == 0) {
        sie::cout << "Can't load kernel\n";
        clReleaseProgram(program);
        clReleaseMemObject(input_buffer);
        clReleaseMemObject(output_graybuff);
        clReleaseCommandQueue(commandQueue);
        clReleaseContext(context);
        return false;
    }
    clEnqueueWriteBuffer(commandQueue, filter_in, CL_TRUE, 0, 3 * 3 * sizeof(float), filter_kernel, 0,
                         NULL,
                         NULL);

    clSetKernelArg(kernel_guss, 0, sizeof(cl_mem), &output_graybuff);
    clSetKernelArg(kernel_guss, 1, sizeof(cl_mem), &output_grayguss);
    clSetKernelArg(kernel_guss, 2, sizeof(cl_mem), &filter_in);
    clSetKernelArg(kernel_guss, 3, sizeof(cl_mem), &imgheight);
    clSetKernelArg(kernel_guss, 4, sizeof(cl_mem), &imgwidth);


    status = clEnqueueNDRangeKernel(commandQueue, kernel_guss,  //启动内核
                                    2, 0, globalThreads, localThreads,
                                    0, NULL, &evt);  //内核执行完成后，会将evt置为CL_SUCCESS/CL_COMPLETE
    clWaitForEvents(1, &evt);   //等待命令事件发生
    clReleaseEvent(evt);

    status = clEnqueueReadBuffer(commandQueue, output_grayguss,
                                 CL_TRUE,
                                 0,
                                 width * height,
                                 img, 0, NULL, NULL);

    sie::cout << " OPenCL 高斯平滑用时(ms):" << timer.nsecsElapsed() /
              (1000.0 * 1000.0) << std::endl;

    ret = stbi_write_bmp("package_gray_guss_cl.bmp", width, height, 1, img);

    if (!ret) {
        std::cout << "write image:"  << " failed."
                  << std::endl;
    }
    delete []img;
    //release device
    clReleaseKernel(kernelrgb2gray);
    clReleaseKernel(kernel_guss);
    clReleaseProgram(program);
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_graybuff);
    clReleaseMemObject(output_grayguss);
    clReleaseMemObject(imgwidth);
    clReleaseMemObject(imgheight);
    clReleaseContext(context);

    if (devices != NULL) {
        free(devices);
        devices = NULL;
    }
    return true;
}

int main(int argc, char *argv[])
{
    std::string filename = "package.bmp";
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        std::cout << "load img [" << filename << "]" <<
                  " width = " << width <<
                  " height = " << height <<
                  " channels = " << nrChannels <<
                  " size = " << width *height *nrChannels << " byte." <<
                  std::endl;
    } else {
        std::cout << "load img " << filename << "failed." << std::endl;
        return 0;
    }

    dealnormal(data, width, height, nrChannels);
    dealcl(data, width, height, nrChannels);
    stbi_image_free(data);
    return 0;
}
