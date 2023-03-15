#include "clr2.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <math.h>
#include <cstring>


cl_device_id *CLR2::get_cl_device_id(cl_platform_id &platform, cl_uint &num_devices)
{
    cl_device_id *devices = NULL;
    cl_int    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    if (num_devices > 0) {
        devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
    }
    return devices;
}

cl_platform_id *CLR2::get_cl_Platforms(cl_uint &num_platform)
{
    cl_int  status = clGetPlatformIDs(0, NULL, &num_platform);
    if (status != CL_SUCCESS) {
        std::cout << "error: Getting platforms failed." << std::endl;
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

cl_program CLR2::load_program(cl_context context, const char *filename, cl_uint count)
{
    std::ifstream in(filename, std::ios_base::binary);

    if (!in.good()) {
        return 0;
    }
    // get file length
    in.seekg(0, std::ios_base::end);

    size_t length = in.tellg();

    in.seekg(0, std::ios_base::beg);

    // read program source
    std::vector<char> data(length + 1);

    in.read(&data[0], length);

    data[length] = 0;

    // create and build program
    const char *source = &data[0];

    cl_program program = clCreateProgramWithSource(context, count, &source, 0, 0);

    if (program == 0) {
        return 0;
    }
    if (clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
        return 0;
    }
    return program;
}

