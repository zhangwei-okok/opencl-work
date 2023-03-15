#ifndef CLR2_H
#define CLR2_H

#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

class CLR2
{
public:
    static cl_platform_id *get_cl_Platforms(cl_uint &num_platform);
    static cl_device_id *get_cl_device_id(cl_platform_id &platform, cl_uint &num_devices);
    static cl_program load_program(cl_context context, const char *filename,cl_uint count = 1);

};
#endif
