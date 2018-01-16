//
//  gauss_blur.h
//  GaussBlur
//
//  Created by wonderidea on 1/16/18.
//  Copyright © 2018 wonderidea. All rights reserved.
//

#ifndef gauss_blur_h
#define gauss_blur_h

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * 对bitmap进行高斯模糊处理
 * @param src_bitmap 原位图数据
 * @param dest_bitmap 目标位图数据
 * @param width 宽
 * @param height 高
 * @param bitCount 色彩位数（支持24位和32位）
 * @param sigma 模糊参数（默认=1.0）
 */
extern int bitmap_gauss_blur(char *src_bitmap,char *dest_bitmap,int width,int height,int bitCount,double sigma);
    
#ifdef __cplusplus
}
#endif

#endif /* gauss_blur_h */
