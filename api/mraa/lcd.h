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
mraa_result_t Draw_X_8Bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);
mraa_result_t Draw_X_8Bit_(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);
mraa_result_t Draw_Y_8Bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color);
int mraa_lcd_read(mraa_lcd_context dev, char* buf, size_t length);

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
