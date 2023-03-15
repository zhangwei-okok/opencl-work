#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "fdoutbuf.h"
#include <cstring>
#include <math.h>
#include "clr2.h"
#include "clelapsedtimer.h"
#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif
#include "cmdline.h"
#include "functioninvoker.h"
#include "objectfactory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define SIZE(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

using namespace std;

std::vector<FunctionInvoker *> functionList;

void loadFunction()
{
    auto itr = ObjectFactory<FunctionInvoker, std::string>::creator->begin();
    while (itr != ObjectFactory<FunctionInvoker, std::string>::creator->end()) {
        FunctionInvoker *node = ObjectFactory<FunctionInvoker, std::string>::create(itr->first);
        functionList.push_back(node);
        itr ++;
    }
}
void diplayFunInfo()
{
    std::vector<FunctionInvoker *>::iterator it;
    for (it = functionList.begin(); it != functionList.end(); it++) {

        std::cout << (*it)->cmdName() << " - " << (*it)->describe()
                  << std::endl;
    }
}

int main(int argc, char *argv[])
{
    loadFunction();
    cmdline::parser parser;
    parser.set_program_name("imgmirror");
    parser.add<std::string>("benchmark", 'b', "A benchmark or options to run: '--benchmark cl'",
                            false);
    parser.add("help", 'h', "Display help");
    parser.add("list-scenes", 'l', "Display information about the available scenes");

    parser.parse_check(argc, argv);
    if (parser.exist("list-scenes")) {
        diplayFunInfo();
        return 1;
    }

    string cmd = "normal";
    if (parser.exist("benchmark"))
        cmd = parser.get<std::string>("benchmark");


    bool bdeal = false;
    std::vector<FunctionInvoker *>::iterator it;
    for (it = functionList.begin(); it != functionList.end(); it++) {

        std::string _cmd = (*it)->cmdName();
        if (_cmd == cmd) {

            // do process
            int width, height, nrChannels;
            unsigned char *data = stbi_load("tgl.bmp", &width, &height, &nrChannels, 0);
            if (data) {
                std::cout << "[load img]" <<
                          " width = " << width <<
                          " height = " << height <<
                          " channels = " << nrChannels <<
                          " size = " << width *height *nrChannels << " byte" <<
                          std::endl;
            } else {
                std::cout << "load img tgl failed." << std::endl;
                return 0;
            }

            unsigned char *img = new unsigned char[width * height * nrChannels];

            sie::cout << "开始处理图像 [" << cmd << "]" << std::endl;
            bool bret = (*it)->process(data, img, width, height, nrChannels);
            if (bret) {
                std::string file = "tgl_mirror_" + _cmd + ".bmp";
                int ret = stbi_write_bmp(file.c_str(), width, height, nrChannels, img);
                if (!ret) {
                    sie::cout << "写图片:"  << file << " 失败."
                              << std::endl;
                }
                sie::cout << "写图片:"  << file << " 成功."
                          << std::endl;
            }
            delete []img;
            bdeal = true;
            stbi_image_free(data);
            break;
        }
    }
    if (!bdeal) {
        std::cout << " can not find cmd[" << cmd << "]" << std::endl;
    }

    return 0;
}
