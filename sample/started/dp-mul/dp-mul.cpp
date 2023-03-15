#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <random>
#include "clr2.h"
#include "clelapsedtimer.h"
#include "fdoutbuf.cpp"

int main(int argc, char *argv[])
{
    cl_int    status;
    // ------------------setup-----------------------//
    //1 获取CL 平台信息
    cl_uint num_platforms;
    cl_platform_id *platforms  = CLR2::get_cl_Platforms(num_platforms);
    if (platforms == nullptr) {
        return 0;
    }
    // 2  获取 平台下的设备信息
    cl_uint num_devices;
    cl_device_id *devices = CLR2::get_cl_device_id(platforms[0], num_devices);
    if (devices == nullptr)
        return 0;

    // 3 为设备通信创建上下文
    cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

    // 4 为设备创建通信的消息队列
    cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

    if (commandQueue == 0) {
        sie::cout << "Can't create command queue\n";
        clReleaseContext(context);
        return 0;
    }

    // 5 准备测试数据，并在OpenCL 设备中开辟对应空间
    const int DATA_SIZE = 1000 * 1000 * 10;

    CLElapsedTimer timer;
    timer.start();
    std::vector<cl_float> a(DATA_SIZE), b(DATA_SIZE), res(DATA_SIZE);
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(1, 200);
    for (int i = 0; i < DATA_SIZE; i++) {
        a[i] =  distr(eng);
        b[i] =  distr(eng);
    }

    sie::cout << "init time(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) << std::endl;

    cl_mem cl_a  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(cl_float) * DATA_SIZE, &a[0], NULL);
    cl_mem cl_b  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(cl_float) * DATA_SIZE, &b[0], NULL);
    cl_mem cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * DATA_SIZE, NULL,
                                   NULL);

    if (cl_a == 0 || cl_b == 0 || cl_res == 0) {
        sie::cout << "Can't create OpenCL buffer\n";
        clReleaseMemObject(cl_a);
        clReleaseMemObject(cl_b);
        clReleaseMemObject(cl_res);
        clReleaseContext(context);
        return 0;
    }

    // 6 根据 Kernel 文件 和 context 生成 program
    cl_program program = CLR2::load_program(context, "dp-mul_kernel.cl");
    if (program == 0) {
        sie::cout << "load program failed." << std::endl;
    }

    //7 根据Kernel 函数名创建一个kernel
    cl_kernel kernel = clCreateKernel(program, "dp_mul", NULL);
    if (kernel == 0) {
        sie::cout << "Can't load kernel\n";
        clReleaseProgram(program);

        clReleaseMemObject(cl_a);

        clReleaseMemObject(cl_b);

        clReleaseMemObject(cl_res);

        clReleaseCommandQueue(commandQueue);

        clReleaseContext(context);
        return 0;
    }
    // 8 设置 Kernel 参数
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &cl_a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &cl_b);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &cl_res);

    sie::cout << "测试 数据大小:" << DATA_SIZE << std::endl;

    timer.restart();
    // 9 发送消息到创建好的消息队列
    size_t global_work_size[1] = {DATA_SIZE};
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL,
                                    &enentPoint);

    clWaitForEvents(1, &enentPoint);
    clReleaseEvent(enentPoint);
    sie::cout << "gpu use time(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) ;
    // 10 等待消息执行结束，从CL 设备缓冲区读取计算结果
    if (status == CL_SUCCESS) {
        status = clEnqueueReadBuffer(commandQueue, cl_res, CL_TRUE, 0, sizeof(float) * DATA_SIZE, &res[0],
                                     0, 0,
                                     0);
    }

    //check opencl  result.
    if (status == CL_SUCCESS) {
        bool correct = true;
        for (int i = 0; i < DATA_SIZE; i++) {

            if (a[i] + b[i] != res[i]) {
                correct = false;
                break;
            }
        }
        if (correct) {
            sie::cout << ",CL 计算正确\n";
        } else {
            sie::cout << ",CL 计算错误.\n";
        }

    } else {
        sie::cout << ",Can't run kernel or read back data\n";
    }

    timer.restart();
    float fc = 0;
    for (int i = 0; i < DATA_SIZE; i++) {
        fc = a[i] + b[i];
    }
    std::cout << "cpu time(ms):" << timer.nsecsElapsed() / (1000.0 * 1000.0) << std::endl;


    //11 释放资源
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(cl_a);
    clReleaseMemObject(cl_b);
    clReleaseMemObject(cl_res);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    if (devices != NULL) {
        free(devices);
        devices = NULL;
    }
    return 0;
}
