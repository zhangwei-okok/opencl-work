const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

kernel void mirrorimg2d(read_only image2d_t src,write_only image2d_t dst)
{
     int x = get_global_id(0);
     int y = get_global_id(1);
     int width = get_image_width(src);
     int4 src_val = read_imagei(src, sampler,
     (int2)(width-1-x, y));
     write_imagei(dst, (int2)(x, y), src_val);
}
