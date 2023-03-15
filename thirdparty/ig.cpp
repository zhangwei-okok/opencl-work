#include "ig.h"
#include <math.h>
#include <cstring>
#include <assert.h>

// 图像水平镜像
void Ig::levelmirror( unsigned char *inbuf,
                      unsigned char *outbuf,
                      int width, int height,
                      int nchannel)
{
    const int bitCount = 8;
    int lineByte = width * (bitCount / 8 + 3) / 4 * nchannel;
    for (int i = 0; i < height ; i++) {
        for (int j = 0; j < lineByte ; j += nchannel) {
            if (nchannel == 3) {
                *(outbuf  + i * lineByte + j) = *( inbuf + i * lineByte + (lineByte - j - 1 - 2));
                *(outbuf  + i * lineByte + j + 1) = *( inbuf + i * lineByte + (lineByte - j - 1 - 1));
                *(outbuf  + i * lineByte + j + 2) = *( inbuf + i * lineByte + (lineByte - j - 1 - 0));

            } else if (nchannel == 4) {
                *(outbuf  + i * lineByte + j) = *( inbuf + i * lineByte + (lineByte - j - 1 - 3));
                *(outbuf  + i * lineByte + j + 1) = *( inbuf + i * lineByte + (lineByte - j - 1 - 2));
                *(outbuf  + i * lineByte + j + 2) = *( inbuf + i * lineByte + (lineByte - j - 1 - 1));
                *(outbuf  + i * lineByte + j + 3) = *( inbuf + i * lineByte + (lineByte - j - 1 - 0));
            }
        }
    }
}

//图像旋转
void Ig::imgrotate(unsigned char *psrc, int srcw, int srch,
                   unsigned char *pdst, int dstw, int dsth,
                   double degree, int nchannel)
{

    int k;
    double angle = degree  * 3.1415926 / 180.;  //旋转角度
    double co = cos(angle); //余弦
    double si = sin(angle); //正弦
    int rotateW, rotateH;   //旋转后图像的高宽
    int srcWidthStep = srcw * nchannel; //宽度步长
    int dstWisthStep = srch * nchannel;
    int x, y;
    int xMin, xMax, yMin, yMax;
    int xOff, yOff; //偏移
    double xSrc = 0.;
    double ySrc = 0.; //变换后图像的坐标在原图中的坐标

    //临时变量
    float valueTemp = 0.;
    float a1, a2, a3, a4;

    memset(pdst, 0, dstWisthStep * dsth * sizeof(unsigned char));
    //计算旋转后的坐标范围
    rotateH = srcw * fabs(si) + srch * fabs(co);
    rotateW = srcw * fabs(co) + srch * fabs(si);

    //计算偏移
    xOff = dstw / 2;
    yOff = dstw / 2;

    yMin = (dsth - rotateH) / 2.;
    yMax = yMin + rotateH + 1; //加1
    xMin = (dsth - rotateW) / 2.;
    xMax = xMin + rotateW + 1;

    for (y = yMin; y <= yMax; y++) {
        for (x = xMin; x <= xMax; x++) {
            //求取在原图中的坐标
            ySrc = si * double(x - xOff) + co * double(y - yOff) + double(int(srch / 2));
            xSrc = co * double(x - xOff) - si * double(y - yOff) + double(int(srcw / 2));

            //如果在原图范围内
            if (ySrc >= 0. && ySrc < srch - 0.5 && xSrc >= 0. && xSrc < srcw - 0.5) {
                //插值
                int xSmall = floor(xSrc);
                int xBig = ceil(xSrc);
                int ySmall = floor(ySrc);
                int yBig = ceil(ySrc);

                for (k = 0; k < nchannel; k++) {
                    a1 = (xSmall >= 0 && ySmall >= 0 ? psrc[ySmall * srcWidthStep + xSmall * nchannel + k] : 0);
                    a2 = (xBig < srcw && ySmall >= 0 ? psrc[ySmall * srcWidthStep + xBig * nchannel + k] : 0);
                    a3 = (xSmall >= 0 && yBig < srch ? psrc[yBig * srcWidthStep + xSmall * nchannel + k] : 0);
                    a4 = (xBig < srcw && yBig < srch ? psrc[yBig * srcWidthStep + xBig * nchannel + k] : 0);
                    double ux = xSrc - xSmall;
                    double uy = ySrc - ySmall;
                    //双线性插值
                    valueTemp = (1 - ux) * (1 - uy) * a1 + (1 - ux) * uy * a3 + (1 - uy) * ux * a2 + ux * uy * a4;
                    pdst[y * dstWisthStep + x * nchannel + k] = floor(valueTemp);
                }

            }
        }
    }
}

void Ig::rgb2Gray(unsigned char *src, unsigned char *dst, const int width, const int height,
                  const int channel)
{
    assert(src);
    assert(dst);
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            int index = h * width + w;

            dst[index] = 0.299f * src[index * channel] +
                         0.587f * src[index * channel + 1] +
                         0.114f * src[index * channel + 2];
        }
    }
}

//空间域 卷积 仅支持灰度图
void Ig::filter2D(unsigned char *srcgray, unsigned char *dstgray, const int height, const int width,
                  int ch,  float kernel[][3])
{
    assert(srcgray);
    assert(dstgray);

    const int channel = 1;
    const int bitCount = 8;
    int lineBytes = width * (bitCount / 8 + 3) / 4 * channel;
    unsigned char *p_src = srcgray;
    unsigned char *p_dst = dstgray;

    int ival = 0;
    for (int irow = 1; irow < (height - 1); irow++ ) {

        unsigned char *previous = (p_src + (irow - 1) * lineBytes);
        unsigned char *current  = (p_src + irow * lineBytes);
        unsigned char *next     = (p_src + (irow + 1) * lineBytes);
        unsigned char *output   = (p_dst + irow * lineBytes);
        for (int icol = ch; icol < width; icol++) {
            ival = previous[icol - ch] * kernel[0][0] + previous[icol] * kernel[0][1] + previous[icol + ch] *
                   kernel[0][2] +
                   current[icol - ch] * kernel[1][0] + current[icol] * kernel[1][1] + current[icol + ch] * kernel[1][2]
                   +
                   next[icol - ch] * kernel[2][0] + next[icol] * kernel[2][1] + next[icol + ch] * kernel[2][2];
            output[icol] = (ival < 0) ? 0 : (ival > 255) ? 255 : ival;
        }
    }
}
