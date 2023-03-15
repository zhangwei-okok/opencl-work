__kernel void kernel_rgb2gray(__global unsigned char * rgbImage,
                              __global unsigned char * grayImage,
                              __global unsigned * const p_height,
                              __global unsigned * const p_width)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int height = *p_height;
    int width = *p_width;
    if(x < width && y < height)
    {
        int index = y * width + x;
        grayImage[index] =  0.299f*rgbImage[index*3] +
                            0.587f*rgbImage[index*3+1] +
                            0.114f*rgbImage[index*3+2];
    }
}

__kernel void kernel_guss(__global uchar * grayImage,
                          __global uchar * gussImage,
                          __global float* filter_in,
                          __global unsigned * const p_height,
                          __global unsigned * const p_width)
{
    int icol = get_global_id(0);
    int irow = get_global_id(1);
    int height = *p_height;
    int width  = *p_width;
    if((icol < width && icol > 0)&& (irow < (height -1)&& irow >0))
    {
        int lineBytes = width * 1;

        int iprevious = (irow - 1) * lineBytes;
        int current = irow * lineBytes;
        int next = (irow + 1) * lineBytes;

        int  ival = grayImage[iprevious + icol -1] * filter_in[0] + grayImage[iprevious + icol] * filter_in[1] + grayImage[iprevious +icol + 1] *filter_in[2] +

                    grayImage[current + icol -1] * filter_in[3] + grayImage[current + icol] * filter_in[4] + grayImage[current +icol + 1] * filter_in[5]+
                    grayImage[next + icol -1] * filter_in[6] + grayImage[next + icol] * filter_in[7] + grayImage[next +icol + 1] * filter_in[8];
         gussImage[irow * lineBytes + icol] = (ival < 0) ? 0 : (ival > 255) ? 255 : ival;
    }
}
