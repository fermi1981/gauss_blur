//
//  gauss_blur.c
//  GaussBlur
//
//  Created by wonderidea on 1/16/18.
//  Copyright © 2018 wonderidea. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "gauss_blur.h"

#define SQRT_2PI 2.506628274631

/**
 * 如果需要精准计算，请开启FLOAT_OP，使用浮点运算，重新编译即可
 */
//#define FLOAT_OP 1

static double Gauss(double sigma, double x)
{
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (sigma * SQRT_2PI);
}

static double* GaussAlgorithm(int win_size, double sigma)
{
    int wincenter, x;
    double *kern, sum = 0.0;
    wincenter = win_size / 2;
    kern = (double*)calloc(win_size, sizeof(double));
    
    for (x = 0; x < wincenter + 1; x++)
    {
        kern[wincenter - x] = kern[wincenter + x] = Gauss(sigma, x);
        sum += kern[wincenter - x] + ((x != 0) ? kern[wincenter + x] : 0.0);
    }
    for (x = 0; x < win_size; x++)
        kern[x] /= sum;
    
    return kern;
}

static int win_size(double sigma)
{
    return (1 + (((int)ceil(3 * sigma)) * 2));
}


int bitmap_gauss_blur(char *src_bitmap,char *dest_bitmap,int width,int height,int bitCount,double sigma)
{
    if (!src_bitmap || !dest_bitmap || width<=0 || height<=0) {
        return -1;
    }
    int	row, col, col_r, col_g, col_b, winsize, halfsize, k, count, rows, count1, count2, count3;
#ifdef FLOAT_OP
    int  row_g, row_b, row_r, col_all;
#else
    int  rowd_g, rowd_b, rowd_r, cold_all;
#endif
    unsigned char  r_r, r_b, r_g, c_all;
    char *tmp;
    double *algorithm;
    int line_size;
    
    
    count=0;
    
    int color_bytes = 4;
    if (bitCount) {
        color_bytes = bitCount>>3;
    }
    line_size = width*color_bytes;
    
    if ((line_size % 4) != 0)
    {
        line_size += (4 - (line_size % 4));
    }
    
    winsize = win_size(sigma);
    algorithm = GaussAlgorithm(winsize, sigma);
    if (!algorithm) {
        return -1;
    }
#ifdef FLOAT_OP
    
#else
    int *algorithm_list=(int *)calloc(winsize, sizeof(int));
    
    for (int i=0; i<winsize; i++) {
        double tmp1 = algorithm[i]*256.0;
        algorithm_list[i] = (int)ceil(tmp1);
    }
#endif
    winsize *= color_bytes;
    halfsize = winsize/2;
    
    tmp = (char*)calloc(line_size * height, sizeof(char));
    if (!tmp) {
        free(algorithm);
        return -1;
    }
    
    for (row = 0; row < height; row++)
    {
        col_r = 2;
        col_g = 1;
        col_b = 0;
        for (rows = 0; rows < line_size; rows += color_bytes)
        {
#ifdef FLOAT_OP
            row_r = row_g = row_b = 0.0;
#else
            rowd_r = rowd_g = rowd_b = 0;
#endif
            count1 = count2 = count3 = 0;
            //set start offset from red color , so k=col_r=2
            for (k = 2; k < winsize; k += color_bytes)
            {
                if ((k + col_r - halfsize >= 0) && (k + col_r - halfsize < line_size))
                {
                    r_r = *(src_bitmap + row * line_size + col_r + k - halfsize);
#ifdef FLOAT_OP
                    row_r += ((int)(r_r&0xFF))* algorithm[count1];
#else
                    rowd_r += ((((int)(r_r&0xFF)) * algorithm_list[count1]));
#endif
                    count1++;
                }
                if ((k + col_g - halfsize >= 0) && (k + col_g - halfsize < line_size))
                {
                    r_g = *(src_bitmap + row * line_size + col_g + k - halfsize);
#ifdef FLOAT_OP
                    row_g += ((int)(r_g&0xFF))* algorithm[count2];
#else
                    rowd_g += ((((int)(r_g&0xFF)) * algorithm_list[count2]));
#endif
                    count2++;
                }
                
                if ((k + col_b - halfsize >= 0) && (k + col_b - halfsize < line_size))
                {
                    r_b = *(src_bitmap + row * line_size + col_b + k - halfsize);
#ifdef FLOAT_OP
                    row_b += ((int)(r_b&0xFF))* algorithm[count3];
#else
                    rowd_b += ((((int)(r_b&0xFF)) * algorithm_list[count3]));
#endif
                    count3++;
                }
            }
#ifdef FLOAT_OP
            *(tmp + row * line_size + col_r) = (unsigned char)(ceil(row_r));
            *(tmp + row * line_size + col_g) = (unsigned char)(ceil(row_g));
            *(tmp + row * line_size + col_b) = (unsigned char)(ceil(row_b));
#else
            rowd_r >>=8;
            rowd_g >>=8;
            rowd_b >>=8;
            
            *(tmp + row * line_size + col_r) = rowd_r>255?255:rowd_r;
            *(tmp + row * line_size + col_g) = rowd_g>255?255:rowd_g;
            *(tmp + row * line_size + col_b) = rowd_b>255?255:rowd_b;
#endif
            if (color_bytes==4) {
                *(tmp + row * line_size + 3) = 0xFF;
            }
            col_r += color_bytes;
            col_g += color_bytes;
            col_b += color_bytes;
        }
    }
    
    winsize /= color_bytes;
    halfsize = winsize / 2;
    for (col = 0; col < line_size; col++)
    {
        for (row = 0; row < height; row++)
        {
            if ((color_bytes==4)&&((col&3)==3)) {
                *(dest_bitmap + row * line_size + col) = 0xFF;
            }
            else
            {
#ifdef FLOAT_OP
                col_all = 0.0;
#else
                cold_all = 0;
#endif
                for (k = 0; k < winsize; k++)
                {
                    if ((k + row - halfsize >= 0) && (k + row - halfsize < height))
                    {
                        c_all = *(tmp + (row + k - halfsize) * line_size + col);
#ifdef FLOAT_OP
                        col_all += ((int)c_all) * algorithm[k];
#else
                        cold_all += ((int)c_all) * algorithm_list[k];
#endif
                    }
                }
#ifdef FLOAT_OP
                *(dest_bitmap + row * line_size + col) = (unsigned char)(ceil(col_all));
#else
                cold_all >>= 8;
                if (cold_all>255) {
                    cold_all=255;
                }
                *(dest_bitmap + row * line_size + col) = cold_all;
#endif
            }
        }
    }
    free(tmp);
    free(algorithm);
#ifndef FLOAT_OP
    free(algorithm_list);
#endif
    return 0;
}

