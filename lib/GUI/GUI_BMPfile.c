/*****************************************************************************
* | File      	:   GUI_BMPfile.h
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                   Used to shield the underlying layers of each master
*                   and enhance portability
*----------------
* |	This version:   V1.0
* | Date        :   2022-10-11
* | Info        :   
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "GUI_BMPfile.h"
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"

#include <stdlib.h>	//exit()
#include <string.h> //memset()
#include <math.h> //memset()

#include "f_util.h"
#include "ff.h"

#define GUI_COLORMAP_NUM_COLORS 7
const uint8_t GUI_ColorMap[GUI_COLORMAP_NUM_COLORS][3] = {
        {0, 0, 0}, // Black
        {255, 255, 255}, // White
        {0, 255, 0}, // Green
        {0, 0, 255}, // Blue
        {255, 0, 0}, // Red
        {255, 255, 0}, // Yellow
        {255, 127, 0}, // Orange
};

UBYTE GUI_ReadBmp_RGB_7Color(const char *path, UWORD Xstart, UWORD Ystart)
{
    BMPFILEHEADER bmpFileHeader;  //Define a bmp file header structure
    BMPINFOHEADER bmpInfoHeader;  //Define a bmp info header structure
    
    FRESULT fr;
    FIL fil;
    UINT br;
    printf("open %s", path);
    fr = f_open(&fil, path, FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        printf("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
        return 1;
    }
    
    // Set the file pointer from the beginning
    f_lseek(&fil, 0);
    f_read(&fil, &bmpFileHeader, sizeof(BMPFILEHEADER), &br);   // sizeof(BMPFILEHEADER) must be 14
    if (br != sizeof(BMPFILEHEADER)) {
        printf("f_read bmpFileHeader error\r\n");
        // printf("br is %d\n", br);
        return 1;
    }
    f_read(&fil, &bmpInfoHeader, sizeof(BMPINFOHEADER), &br);   // sizeof(BMPFILEHEADER) must be 50
    if (br != sizeof(BMPINFOHEADER)) {
        printf("f_read bmpInfoHeader error\r\n");
        // printf("br is %d\n", br);
        return 1;
    }
    if(bmpInfoHeader.biWidth > bmpInfoHeader.biHeight)
        Paint_SetRotate(0);
    else
        Paint_SetRotate(90);

    printf("pixel = %d * %d\r\n", bmpInfoHeader.biWidth, bmpInfoHeader.biHeight);

    // Determine if it is a monochrome bitmap
    int readbyte = bmpInfoHeader.biBitCount;
    if(readbyte != 24){
        printf("Bmp image is not 24 bitmap!\n");
        return 1;
    }
    // Read image data into the cache
    UWORD x, y;
    UBYTE Rdata[3];
    
    f_lseek(&fil, bmpFileHeader.bOffset);

    printf("read data\n");
    
    for(y = 0; y < bmpInfoHeader.biHeight; y++) {//Total display column
        for(x = 0; x < bmpInfoHeader.biWidth ; x++) {//Show a line in the line
            if(f_read(&fil, (char *)Rdata, 1, &br) != FR_OK) {
                perror("get bmpdata:\r\n");
                break;
            }
            if(f_read(&fil, (char *)Rdata+1, 1, &br) != FR_OK) {
                perror("get bmpdata:\r\n");
                break;
            }
            if(f_read(&fil, (char *)Rdata+2, 1, &br) != FR_OK) {
                perror("get bmpdata:\r\n");
                break;
            }

            uint8_t pixel_r = Rdata[2], pixel_g = Rdata[1], pixel_b = Rdata[0];
            uint8_t color_min_delta_idx = 0;
            uint32_t color_min_delta = UINT32_MAX;
            for (int color_idx = 0; color_idx < GUI_COLORMAP_NUM_COLORS; color_idx++) {
                uint8_t color_r = GUI_ColorMap[color_idx][0],
                    color_g = GUI_ColorMap[color_idx][1],
                    color_b = GUI_ColorMap[color_idx][2];
                uint32_t color_delta = 0;

                uint32_t tmp_color_accu = color_r-pixel_r;
                color_delta += tmp_color_accu*tmp_color_accu;

                tmp_color_accu = color_g-pixel_g;
                color_delta += tmp_color_accu*tmp_color_accu;

                tmp_color_accu = color_b-pixel_b;
                color_delta += tmp_color_accu*tmp_color_accu;

                if (color_delta >= color_min_delta)
                    continue;

                color_min_delta = color_delta;
                color_min_delta_idx = color_idx;

                if (color_min_delta == 0)
                    break;
            }

            Paint_SetPixel(Xstart + bmpInfoHeader.biWidth-1-x, Ystart + y, color_min_delta_idx);
        }
        watchdog_update();
    }
    printf("close file\n");
    f_close(&fil);

    return 0;
}


