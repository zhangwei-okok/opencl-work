__kernel void mirror(__global uchar * src_data,
                     __global uchar * dest_data,
                     __global unsigned * const p_width,
                     __global unsigned * const p_height,
                     __global unsigned * const p_channel)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    int height = *p_height;
    int width =  *p_width;
    int channel = *p_channel;
    if(x < width && y < height)
    {
        int lineByte = width * channel;
        if(channel == 3){
            *(dest_data  + y * lineByte + x*channel) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 2));
            *(dest_data  + y * lineByte + x*channel + 1) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 1));
            *(dest_data  + y * lineByte + x*channel + 2) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 0));

        }
        else if(channel == 4)
        {
            *(dest_data  + y * lineByte + x*channel) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 3));
            *(dest_data  + y * lineByte + x*channel + 1) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 2));
            *(dest_data  + y * lineByte + x*channel + 2) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 1));
            *(dest_data  + y * lineByte + x*channel + 3) = *( src_data + y * lineByte + (lineByte - x*channel - 1 - 0));
        }
    }
}
