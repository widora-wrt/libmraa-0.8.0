/*
 * Author: Thomas Ingleby <thomas.c.ingleby@intel.com>
 * Contributions: Jon Trulson <jtrulson@ics.com>
 *                Brendan Le Foll <brendan.le.foll@intel.com>
 * Copyright (c) 2014 - 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/**
 * @file
 * @brief lcd module
 *
 * lcd is the Universal asynchronous receiver/transmitter interface to
 * libmraa. It allows the exposure of lcd pins on supported boards.
 * With functionality to expand at a later date.
 *
 * @snippet lcd.c Interesting
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "common.h"

typedef struct _lcd* mraa_lcd_context;
#define BIT(x) (1<<x)

typedef struct{
	unsigned char Witdh;
	unsigned char High;
	const unsigned char *ELib;
	const unsigned char *EIndex;
	const unsigned char *CLib;
	const unsigned char *CIndex;
}FontTypeStruct;

extern const unsigned char GBK0612Lib[];
extern const unsigned char GBK1212_Index[];
extern const unsigned char GBK1212Lib[];

extern const unsigned char GBK0816Lib[];
extern const unsigned char GBK1616_Index[];
extern const unsigned char GBK1616Lib[];

extern const unsigned char GBK0824Lib[];
extern const unsigned char GBK1624_Index[];
extern const unsigned char GBK1624Lib[];

extern const unsigned char GBK1224Lib[];
extern const unsigned char GBK2424_Index[];
extern const unsigned char GBK2424Lib[];

extern const unsigned char GBK1632Lib[];
extern const unsigned char GBK3232_Index[];
extern const unsigned char GBK3232Lib[];

extern const unsigned char GBK2448Lib[];
extern const unsigned char GBK4848_Index[];
extern const unsigned char GBK4848Lib[];


extern const FontTypeStruct GBKLib_FontType1224;
extern const FontTypeStruct GBKLib_FontType1616;
extern const FontTypeStruct GBKLib_FontType1624;
extern const FontTypeStruct GBKLib_FontType3232;
extern const FontTypeStruct GBKLib_FontType2424;
extern const FontTypeStruct GBKLib_FontType4848;

const FontTypeStruct FontGetType(unsigned short f);

extern const unsigned int gb2312_utf8_code[][2];
#define uint8 unsigned char 
#define uint16 unsigned short 
#define uint32 unsigned int 
#define RGB8882RGB565(rgb888)   ((((rgb888)&0xf80000)>>8)|(((rgb888)&0xfc00)>>5)|(((rgb888)&0xf8)>>3))
#define RGB565_MASK_RED        0xF800  
#define RGB565_MASK_GREEN    0x07E0  
#define RGB565_MASK_BLUE       0x001F  
/**
 * Initialise lcd_context, uses board mapping
 *
 * @param lcd the index of the lcd set to use
 * @return lcd context or NULL
 */
mraa_lcd_context mraa_lcd_init(int lcd);

/**
 * Initialise a raw lcd_context. No board setup.
 *
 * @param path for example "/dev/ttyS0"
 * @return lcd context or NULL
 */
mraa_lcd_context mraa_lcd_init_raw(const char* path);

/**
 * Destroy a mraa_lcd_context
 *
 * @param dev lcd context
 * @return mraa_result_t
 */
mraa_result_t mraa_lcd_stop(mraa_lcd_context dev);

/**
 * Read bytes from the device into a buffer
 *
 * @param dev lcd context
 * @param buf buffer pointer
 * @param length maximum size of buffer
 * @return the number of bytes read, or -1 if an error occurred
 */
mraa_result_t mraa_lcd_drawdot(mraa_lcd_context dev,unsigned int x,unsigned int y,unsigned short color);
mraa_result_t mraa_lcd_drawline(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short color);
mraa_result_t mraa_lcd_drawrect(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short Color);
mraa_result_t mraa_lcd_drawrectfill(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short color);
mraa_result_t mraa_lcd_drawcircle(mraa_lcd_context dev,int x0,int y0,int r,int color);
mraa_result_t mraa_lcd_drawcirclefill(mraa_lcd_context dev,unsigned int x0,unsigned int y0,unsigned int r,unsigned short color);
mraa_result_t mraa_lcd_drawclear(mraa_lcd_context dev,unsigned short c);
mraa_result_t mraa_lcd_draw_x_8Bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);
mraa_result_t mraa_lcd_draw_x_8Bit_(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);
mraa_result_t mraa_lcd_draw_y_8Bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);


mraa_result_t mraa_drawfont_ascii(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,unsigned short Char,unsigned short f_color,unsigned short b_color,unsigned short a_color);
mraa_result_t mraa_lcd_drawfont_string(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,const unsigned char *Str,unsigned short f_color,unsigned short b_color,unsigned short a_color);
mraa_result_t mraa_drawfont_word(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,const unsigned char *word,unsigned short f_color,unsigned short b_color,unsigned short a_color);
mraa_result_t mraa_lcd_drawjpg(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name);
mraa_result_t mraa_lcd_drawpng(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name);
int mraa_lcd_read(mraa_lcd_context dev, char* buf, size_t length);
mraa_result_t mraa_lcd_writeline(mraa_lcd_context dev, const char* buf);
mraa_result_t mraa_lcd_drawdotaraay(mraa_lcd_context dev,  uint8_t* data,int length,int cf,int cb);
mraa_result_t mraa_lcd_drawdotaraaybit(mraa_lcd_context dev,int x,int y,char color);
mraa_result_t mraa_lcd_drawdotaraaymove(mraa_lcd_context dev,int x,int y);
char * mraa_lcd_screenshotsave(mraa_lcd_context dev);
mraa_result_t mraa_lcd_screenshotdebug(mraa_lcd_context dev,char * name);
mraa_result_t mraa_lcd_screenprevie(mraa_lcd_context dev,char * name);
mraa_result_t mraa_lcd_screenstream(mraa_lcd_context dev,char * name);
mraa_boolean_t mraa_lcd_getdotaraaybit(mraa_lcd_context dev,int x,int y);
unsigned short mraa_lcd_getdot(mraa_lcd_context dev,unsigned int x,unsigned int y);
mraa_result_t mraa_lcd_selectaraaydot(mraa_lcd_context dev, int x,int y,int color);
mraa_result_t mraa_lcd_drawpic(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name);
mraa_result_t mraa_lcd_drawfreetype_string(mraa_lcd_context dev,uint16 size,uint16 x,uint16 y,const uint8 *str,int color_f,int color_b,int color_a);
mraa_result_t mraa_lcd_drawawesome_index(mraa_lcd_context dev,uint16 size,uint16 x,uint16 y,int ind,int color_f,int color_b,int color_a);
mraa_result_t mraa_lcd_filltriangle(mraa_lcd_context dev,int x0, int y0, int x1, int y1, int x2, int y2, int color);
/**
 * Write bytes in buffer to a device
 *
 * @param dev lcd context
 * @param buf buffer pointer
 * @param length maximum size of buffer
 * @return the number of bytes written, or -1 if an error occurred
 */
int mraa_lcd_write(mraa_lcd_context dev, const char* buf, size_t length);

#ifdef __cplusplus
}
#endif
