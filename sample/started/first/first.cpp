#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "fdoutbuf.h"
#include <cstring>
#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

using namespace std;
cl_device_id *get_cl_device_id(cl_platform_id &platform, cl_uint &num_devices)
{
    cl_device_id *devices = NULL;
    cl_int    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    if (num_devices > 0) {
        devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
    }
    return devices;
}

cl_platform_id *get_cl_Platforms(cl_uint &num_platform)
{
    cl_int    status = clGetPlatformIDs(0, NULL, &num_platform);
    if (status != CL_SUCCESS) {
        sie::cout << "error: Getting platforms failed." << std::endl;
        return nullptr;
    }
    cl_platform_id *platforms = nullptr;
    if (num_platform > 0) {
        platforms =
            (cl_platform_id * )malloc(num_platform * sizeof(cl_platform_id));
        clGetPlatformIDs(num_platform, platforms, NULL);
    }
    return platforms;
}

int main(int argc, char *argv[])
{
    cl_uint num_platforms;
    cl_platform_id *platforms  = get_cl_Platforms(num_platforms);
    if (num_platforms > 0) {
        sie::cout << "CL 平台个数: " << num_platforms << std::endl;
        for (cl_uint i = 0; i < num_platforms; i++) {

            sie::cout << "--------------------------------------" << std::endl;
            //显示平台具体信息
            char info_str[256];
            memset(&info_str, '\0', 256);
            cl_int  ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME,
                                            sizeof(info_str), info_str, NULL);
            if (ret != CL_SUCCESS) {
                sie::cout << "get platform info failed." << std::endl;
            }
            sie::cout << "OpenCL 平台[ " << i << " ]" << info_str << std::endl;

            ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION,
                                    sizeof(info_str), info_str, NULL);
            sie::cout << "version: " << info_str << std::endl;

            //平台对应设备信息
            cl_uint num_devices;
            cl_device_id *device_ids =  get_cl_device_id(platforms[i], num_devices);
            if (num_devices <= 0)
                break;

            for (int i = 0; i < num_devices; ++i) {
                size_t dev_param_size = 0;
                clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME,
                                sizeof(info_str), info_str, &dev_param_size);
                sie::cout << "device:" <<  info_str << std::endl;
                size_t local_size = 0;
                clGetDeviceInfo(device_ids[i], CL_DEVICE_LOCAL_MEM_SIZE,
                                sizeof(local_size), &local_size, &dev_param_size);
                sie::cout << "Block mem size: " << local_size / 1024.0 << std::endl;

            }
            if (device_ids != NULL) {
                free(device_ids);
            }
        }
    }
    if (platforms != nullptr) {
        free(platforms);
    }
    return 0;
}
