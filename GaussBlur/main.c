//
//  main.c
//  GaussBlur
//
//  Created by wonderidea on 1/15/18.
//  Copyright © 2018 wonderidea. All rights reserved.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
//#include <conio.h>
//#include <malloc.h>
#include "gauss_blur.h"

#pragma pack(push,2)
typedef struct
{
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount; // глубина цвета
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} bmp_header_t;
#pragma pack(pop)

typedef struct
{
    bmp_header_t header;
    char *data;
} bmp_t;

int win_size(double);
double Gauss(double, double);
double *GaussAlgorithm(int, double);
void bmp_free(bmp_t *);
bmp_t *bmp_open(FILE *);
int bmp_write(bmp_t*, FILE*);
bmp_t *GaussBlur(bmp_t*, double);


const char *usage_string[]={
    "usage:",
    "exe {input bmpfile} {output bmp file} {options}",
    "options:",
    "-s|--sigma:set sigma value",
    "",
};


int main(int argc, const char * argv[])
{
    int help = 1;
    if (argc>=3) {
        
        
        FILE *InputFile, *OutputFile;
        bmp_t *bmp = NULL, *blur = NULL;
        
        //const char *InputName = "/Users/wonderidea/Documents/before_gauss.bmp";
        //const char *OutputName = "/Users/wonderidea/Documents/after_gauss.bmp";
        //const char *InputName = "/Users/wonderidea/Documents/test_gauss_blur.bmp";
        //const char *OutputName = "/Users/wonderidea/Documents/tt_after_gauss.bmp";
        
        const char *InputName = argv[1];
        const char *OutputName = argv[2];
        double sigma = 1.0;
        if (argc>3) {
            int index = 3;
            while (index<argc) {
                if (strcmp(argv[index], "-s")==0 || strcmp(argv[index], "--sigma")==0) {
                    index++;
                    sigma = atof(argv[index]);
                    index++;
                }
            }
        }
        
        InputFile = fopen(InputName, "rb");
        if (InputFile==NULL) {
            return 1;
        }
        bmp = bmp_open(InputFile);
        fclose(InputFile);
        
        OutputFile = fopen(OutputName, "wb");
        if (OutputFile == NULL) {
            //fprintf(stderr, "Can't open %s\n", OutputName);
            return EXIT_FAILURE;
        }
        blur = GaussBlur(bmp, sigma);
        bmp_write(blur, OutputFile);
        fclose(OutputFile);
        bmp_free(bmp);
        free(blur);
        //getchar();
        help = 0;
    }
    
    if (help) {
        int count = sizeof(usage_string)/sizeof(usage_string[0]);
        for (int i=0; i<count; i++) {
            printf("%s\n",usage_string[i]);
        }
    }
    
    return 0;
}

bmp_t *GaussBlur(bmp_t *src, double sigma)
{
    int width, height;
    int line_size;
    int color_bytes = 4;
    if (src->header.biBitCount) {
        color_bytes = src->header.biBitCount>>3;
    }
    width = src->header.biWidth;
    height = src->header.biHeight;
    line_size = width*color_bytes;
    
    int bl_flag = 0;
    if (height<0) {
        height = 0-height;
        bl_flag = 1;
    }
    
    if ((line_size % 4) != 0)
    {
        line_size += (4 - (line_size % 4));
    }
    
    bmp_t *blur;
    blur = (bmp_t*)malloc(sizeof(bmp_t));
    blur->header = src->header;
    blur->header.biWidth = src->header.biWidth;
    if (bl_flag)
    {
        blur->header.biHeight = 0-height;
    }
    else
    {
        blur->header.biHeight = height;
    }
    blur->header.biSizeImage = line_size * height;
    blur->data = (char*)malloc(blur->header.biSizeImage);
    int ret=bitmap_gauss_blur(src->data, blur->data, width, height, src->header.biBitCount, sigma);
    if (ret<0) {
        printf("bitmap gauss blur failed!\n");
    }
    return blur;
}

bmp_t *bmp_open(FILE *f) {
    bmp_t *bmp;
    bmp = (bmp_t *)malloc(sizeof(bmp_t));
    bmp->data = NULL;
    //printf("bmp_header_t=%lu\n",sizeof(bmp_header_t));
    if (fread(&(bmp->header), sizeof(bmp_header_t), 1, f))
    {
        int w=bmp->header.biWidth;
        int h=bmp->header.biHeight;
        if (h<0) {
            h = 0-h;
        }
        int color_bytes=4;
        if (bmp->header.biBitCount) {
            color_bytes = bmp->header.biBitCount>>3;
        }
        if (bmp->header.biSizeImage==0) {
            bmp->header.biSizeImage = w*h*color_bytes;
        }
        bmp->data = (char*)malloc(bmp->header.biSizeImage);
        if (fread(bmp->data, bmp->header.biSizeImage, 1, f))
            return bmp;
    }
    //fprintf(stderr, "Error reading file");
    bmp_free(bmp);
    return NULL;
}

int bmp_write(bmp_t *bmp, FILE *out) {
    
    int w=bmp->header.biWidth;
    int h=bmp->header.biHeight;
    if (h<0) {
        h = 0-h;
    }
    int color_bytes=4;
    if (bmp->header.biBitCount) {
        color_bytes = bmp->header.biBitCount>>3;
    }
    bmp->header.biSizeImage = w*h*color_bytes;
    
    //bmp->header.biHeight = 0-h;
    fwrite(&(bmp->header), sizeof(bmp_header_t), 1, out);
    fwrite(bmp->data, bmp->header.biSizeImage, 1, out);
    return 0;
}

void bmp_free(bmp_t *bmp)
{
    if (bmp == NULL) return;
    if (bmp->data != NULL) free(bmp->data);
    free(bmp);
}


