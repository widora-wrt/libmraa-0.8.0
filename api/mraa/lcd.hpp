/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Contributions: Jon Trulson <jtrulson@ics.com>
 * Contributions: Thomas Ingleby <thomas.c.ingleby@intel.com>
 * Copyright (c) 2014 - 2015 Intel Corporation.
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

#include "lcd.h"
#include "types.hpp"
#include <stdexcept>
#include <cstring>

namespace mraa
{

/**
 * @brief API to UART (enabling only)
 *
 * This file defines the UART interface for libmraa
 *
 * @snippet Uart-example.cpp Interesting
 */
class Lcd
{
  public:
    /**
     * Uart Constructor, takes a pin number which will map directly to the
     * linux uart number, this 'enables' the uart, nothing more
     *
     * @param uart the index of the uart set to use
     */
    Lcd(int lcd)
    {
        m_lcd = mraa_lcd_init(lcd);

        if (m_lcd == NULL) {
            throw std::invalid_argument("Error initialising lcd");
        }
    }

    /**
     * lcd Constructor, takes a string to the path of the serial
     * interface that is needed.
     *
     * @param lcd the index of the lcd set to use
     */
    Lcd(std::string path)
    {
        m_lcd = mraa_lcd_init_raw(path.c_str());

        if (m_lcd == NULL) {
            throw std::invalid_argument("Error initialising lcd");
        }
    }

    /**
     * Uart destructor
     */
    ~Lcd()
    {
        mraa_lcd_stop(m_lcd);
    }
    
    Result
    drawDot(float x, float y,std::string colors)
    {
        printf("%s",colors.c_str()+1);
        return (Result) mraa_lcd_drawdot(m_lcd, x, y,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawLine(unsigned int x1, unsigned int y1,unsigned int x2, unsigned int y2,std::string colors)
    {
        return (Result) mraa_lcd_drawline(m_lcd, x1,y1,x2,y2,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    
    Result
    drawRect(unsigned int x1, unsigned int y1,unsigned int x2, unsigned int y2,std::string colors)
    {
        return (Result) mraa_lcd_drawrect(m_lcd, x1,y1,x2,y2,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawRectfill(unsigned int x1, unsigned int y1,unsigned int x2, unsigned int y2,std::string colors)
    {
        return (Result) mraa_lcd_drawrectfill(m_lcd, x1,y1,x2,y2,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawCircle(int x0,int y0,int r,std::string colors)
    {
        return (Result) mraa_lcd_drawcircle(m_lcd, x0,y0,r,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawCirclefill(int x0,int y0,int r,std::string colors)
    {
        return (Result) mraa_lcd_drawcirclefill(m_lcd, x0,y0,r,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawClear(std::string colors)
    {
        return (Result) mraa_lcd_drawclear(m_lcd,mraa_lcd_rgb2tft((int)strtol(colors.c_str()+1,0, 16)));
    }
    Result
    drawString(unsigned short f,unsigned short x,unsigned short y,std::string data,std::string f_color,std::string b_color,std::string a_color)
    {
        return (Result) mraa_lcd_drawfont_string(m_lcd,f,x,y,(const unsigned char *)data.c_str(), \
        mraa_lcd_rgb2tft((int)strtol(f_color.c_str()+1,0, 16)), \
        mraa_lcd_rgb2tft((int)strtol(b_color.c_str()+1,0, 16)), \
        mraa_lcd_rgb2tft((int)strtol(a_color.c_str()+1,0, 16)));
    }
    Result
    drawJpg(unsigned short x,unsigned short y,std::string data)
    {
        return (Result) mraa_lcd_drawjpg(m_lcd,x,y,(const unsigned char *)data.c_str());
    }
    /**
     * Read bytes from the device into char* buffer
     *
     * @param data buffer pointer
     * @param length maximum size of buffer
     * @return numbers of bytes read
     */
    int
    read(char* data, int length)
    {
        return mraa_lcd_read(m_lcd, data, (size_t) length);
    }

    /**
     * Write bytes in String object to a device
     *
     * @param data buffer pointer
     * @param length maximum size of buffer
     * @return the number of bytes written, or -1 if an error occurred
     */
    int
    write(const char* data, int length)
    {
        return mraa_lcd_write(m_lcd, data, (size_t) length);
    }

  private:
    mraa_lcd_context m_lcd;
};
}
