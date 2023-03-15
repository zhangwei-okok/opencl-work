#ifndef IG_DEAL_H
#define IG_DEAL_H

class Ig
{
public:
    //图像水平镜像
    static void levelmirror(unsigned char *inbuf,
                         unsigned char *outbuf,
                         int width, int height,
                         int nchannel);
    //图像旋转
    static void  imgrotate(unsigned char *psrc, int srcw, int srch,
                         unsigned char *pdst, int dstw, int dsth,
                         double degree, int nchannel);

    static void rgb2Gray(unsigned char *src, unsigned char *dst, const int width, const int height,
                      const int channel);
    static void filter2D(unsigned char *srcgray, unsigned char *dstgray, const int height, const int width,
                      int ch,  float kernel[][3]);
};

#endif
