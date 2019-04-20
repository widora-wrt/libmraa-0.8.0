/*
 * Author: Thomas Ingleby <thomas.c.ingleby@intel.com>
 * Contributions: Jon Trulson <jtrulson@ics.com>
 *                Brendan le Foll <brendan.le.foll@intel.com>
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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <png.h>
#include<sys/time.h>
#include <jerror.h> 
#include "lcd.h"
#include "mraa_internal.h"

 #include <sys/socket.h>
 #include <sys/types.h>
 #include <string.h>
 #include <netinet/in.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <sys/time.h>
 #include <pthread.h>
#define MAXLINE 1024
#include <math.h>
#include <ft2build.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdlib.h>
#include FT_FREETYPE_H
#define MAPWIDTH   320
unsigned char image[240][MAPWIDTH];

#define IO_BUFFER 256
#define BOUNDARY "boundarydonotcross"
#define STD_HEADER "Connection: close\r\n" \
    "Server: MJPG-Streamer/0.2\r\n" \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n" \
    "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"

typedef struct {
    int treadfd;           
    mraa_lcd_context dev;
} fbtreadstream;

static mraa_lcd_context
mraa_lcd_init_internal(mraa_adv_func_t* func_table)
{
    mraa_lcd_context dev = (mraa_lcd_context) calloc(1, sizeof(struct _lcd));
    if (dev == NULL) {
        syslog(LOG_CRIT, "uart: Failed to allocate memory for context");
        return NULL;
    }
    dev->index = -1;
    dev->fd = -1;
    dev->advance_func = func_table;
    return dev;
}

mraa_lcd_context
mraa_lcd_init(int index)
{
     mraa_lcd_context dev = mraa_lcd_init_raw((char*)plat->lcd_dev[index].device_path);
    return dev;
}

mraa_lcd_context
mraa_lcd_init_raw(const char* path)
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    FILE* fphzk;
    long int screensize=0,size;
    mraa_lcd_context dev = mraa_lcd_init_internal(plat == NULL ? NULL : plat->adv_func);
    if (dev == NULL) {
        syslog(LOG_ERR, "uart: Failed to allocate memory for context");
        return NULL;
    }

    fphzk= fopen("/www/cgi-bin/opt/font/HZK16", "rb");
    if(fphzk == NULL){
        syslog(LOG_ERR,"Error: not found HZK16");
        free(dev);
        return NULL;
    }
    fseek(fphzk, 0, SEEK_END);
    size = ftell(fphzk);
    fseek(fphzk, 0, SEEK_SET);
    if(size > 0)
    {
      dev->f16p= (char *)calloc(size,sizeof(char));
    }
    fread(dev->f16p, size, 1, fphzk);
    fclose(fphzk);
    dev->path = path;
    if (!dev->path) {
        syslog(LOG_ERR, "lcd: device path undefined, open failed");
        free(dev);
        return NULL;
    }
    // now open the device
    if ((dev->fd = open(dev->path, O_RDWR)) == -1) {
        syslog(LOG_ERR, "lcd: open() failed");
        free(dev);
        return NULL;
    }
    if (ioctl(dev->fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        syslog(LOG_ERR,"Error reading fixed information");
        exit(2);
    }
    if (ioctl(dev->fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        syslog(LOG_ERR,"Error reading variable information");
        free(dev);
        return NULL;
    }
    dev->xres= vinfo.xres;
    dev->yres= vinfo.yres;
    dev->bits_per_pixel= vinfo.bits_per_pixel;
    dev->xoffset= vinfo.xoffset;
    dev->line_length=finfo.line_length;
	screensize = dev->xres * dev->yres * dev->bits_per_pixel / 8;
    dev->fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,dev->fd, 0);
    if ((int)dev->fbp == -1) {
        syslog(LOG_ERR,"Error: failed to map framebuffer device to memory");
        free(dev);
        return NULL;
    }
    return dev;
}

mraa_result_t
mraa_lcd_drawdot(mraa_lcd_context dev,unsigned int x,unsigned int y,unsigned short color)
{
	if(x>=dev->xres)return;
	if(y>=dev->yres)return;
    long int location = (x+dev->xoffset) * (dev->bits_per_pixel/8) +(y+dev->yoffset) *  dev->line_length;
    *((unsigned short int*)(dev->fbp + location)) =color;
    return MRAA_SUCCESS;
}
unsigned short
mraa_lcd_getdot(mraa_lcd_context dev,unsigned int x,unsigned int y)
{
    unsigned short color;
    if(x>=dev->xres)return 0;
	if(y>=dev->yres)return 0;
    long int location = (x+dev->xoffset) * (dev->bits_per_pixel/8) +(y+dev->yoffset) *  dev->line_length;
    color=*((unsigned short int*)(dev->fbp + location));
    return color;
}
mraa_result_t
mraa_lcd_drawline(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short color)
{
    unsigned int t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		mraa_lcd_drawdot(dev,uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawrect(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short Color)
{
	mraa_lcd_drawline(dev,x1,y1,x1,y2,Color);	
	mraa_lcd_drawline(dev,x1,y1,x2,y1,Color);	
	mraa_lcd_drawline(dev,x2,y2,x1,y2,Color);	
	mraa_lcd_drawline(dev,x2,y1,x2,y2,Color);	
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawrectfill(mraa_lcd_context dev,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2,unsigned short color)
{
     int x = 0, y = 0;
     x2++;
     y2++;
     for(x=x1;x<x2;x++)
     for(y=y1;y<y2;y++)
     {
        mraa_lcd_drawdot(dev,x,y,color);
     }
     return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_circle_point(mraa_lcd_context dev,int x,int y,int x0,int y0,int color)
{
	mraa_lcd_drawdot(dev,x+x0,y+y0,color);
	mraa_lcd_drawdot(dev,y+x0,x+y0,color);
	mraa_lcd_drawdot(dev,y+x0,-x+y0,color);
	mraa_lcd_drawdot(dev,x+x0,-y+y0,color);
	mraa_lcd_drawdot(dev,-x+x0,-y+y0,color);
	mraa_lcd_drawdot(dev,-y+x0,-x+y0,color);
	mraa_lcd_drawdot(dev,-y+x0,x+y0,color);
	mraa_lcd_drawdot(dev,-x+x0,y+y0,color);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawcircle(mraa_lcd_context dev,int x0,int y0,int r,int color)
{
	int x,y,d;
	x=0;
	y=r;
	d=1-r;
	mraa_lcd_circle_point(dev,x,y,x0,y0,color);
	while(x<=y)
	{
		if(d<0){d+=2*x+3;x++;}
		else{d+=2*(x-y)+5;x++;y--;}
		mraa_lcd_circle_point(dev,x,y,x0,y0,color);
	}
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_circle_line(mraa_lcd_context dev,int x,int y,int x0,int y0,int color)
{
    mraa_lcd_drawline(dev,x+x0,y+y0,-x+x0,y+y0,color);
    mraa_lcd_drawline(dev,y+x0,-x+y0,-y+x0,-x+y0,color);
    mraa_lcd_drawline(dev,-x+x0,-y+y0,x+x0,-y+y0,color);
    mraa_lcd_drawline(dev,-y+x0,x+y0,y+x0,x+y0,color);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawcirclefill(mraa_lcd_context dev,unsigned int x0,unsigned int y0,unsigned int r,unsigned short color)
{
	int x,y,d;
	x=0;
	y=r;
	d=1-r;
	mraa_lcd_circle_point(dev,x,y,x0,y0,color);
	while(x<=y)
	{
		if(d<0){d+=2*x+3;x++;}
		else{d+=2*(x-y)+5;x++;y--;}
		mraa_lcd_circle_line(dev,x,y,x0,y0,color);
	}
    mraa_lcd_drawline(dev,x0-r,y0,x0+r,y0,color);
    return MRAA_SUCCESS; 
}
mraa_result_t
mraa_lcd_drawclear(mraa_lcd_context dev,unsigned short c)
{
    mraa_lcd_drawrectfill(dev,0,0,dev->xres,dev->yres,c);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_draw_x_8bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color)
{
	char i;
	for(i=0;i<8;i++)
	{
		if(BIT(i)&Data)mraa_lcd_drawdot(dev,X,Y,F_Color);
		else if(B_Color!=A_Color)mraa_lcd_drawdot(dev,X,Y,B_Color);
		X++;
	}
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_draw_x_8bit_(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color)
{
	char i;
	for(i=7;i>-1;i--)
	{
		if(BIT(i)&Data)mraa_lcd_drawdot(dev,X,Y,F_Color);
		else if(B_Color!=A_Color)mraa_lcd_drawdot(dev,X,Y,B_Color);
		X++;
	}
    return MRAA_SUCCESS;
}
/****************************************************************************
Date:2013/8/14
Vision:V1.0
Func:垂直打印一字节显示像素
Note:(Data) 一字节控制像素亮灭 (X,Y) 引用全局地址但不改变全局地址
****************************************************************************/
mraa_result_t
mraa_lcd_draw_y_8bit(mraa_lcd_context dev,unsigned char Data,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color)
{
	char i;
	for(i=0;i<8;i++)
	{
		if(BIT(i)&Data)mraa_lcd_drawdot(dev,X,Y,F_Color);
		else if(B_Color!=A_Color)mraa_lcd_drawdot(dev,X,Y,B_Color);
		Y++;
	}
    return MRAA_SUCCESS;
}

mraa_result_t
mraa_lcd_draw_full_list(mraa_lcd_context dev,void *Data,unsigned short Data_Length,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color)
{
	unsigned short i;
	unsigned char *p;
	p=(unsigned char *)Data;
	for(i=0;i<Data_Length;i++)
	{
		mraa_lcd_draw_x_8bit(dev,*p++,X,Y,F_Color,B_Color,A_Color);
		Y++;
	}
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_draw_full_lists(mraa_lcd_context dev,void *Data,unsigned short w,unsigned short Data_Length,unsigned short X,unsigned short Y,unsigned short F_Color,unsigned short B_Color,unsigned short A_Color)
{
	unsigned short i,n;
	unsigned char *p;
	p=(unsigned char *)Data;
    w/=8;
	for(i=0;i<Data_Length;i++)
	{
        for(n=0;n<w;n++)
        {
		    mraa_lcd_draw_x_8bit_(dev,*p++,X+n*8,Y,F_Color,B_Color,A_Color);
        }
		Y++;
	}
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_stop(mraa_lcd_context dev)
{
	unsigned long screensize;
    if (!dev) {
        syslog(LOG_ERR, "lcd: stop: context is NULL");
        return MRAA_ERROR_INVALID_HANDLE;
    }
    dev->stream_run=0;
	if(dev->f16p!=NULL)
	{
        free(dev->f16p);
    	dev->f16p= NULL;
	}
    if (dev->fd >= 0) {
		screensize = dev->xres * dev->yres * dev->bits_per_pixel / 8;
	    munmap(dev->fbp, screensize);
        close(dev->fd);
    }
    free(dev);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawfont_ascii(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,unsigned short Char,unsigned short f_color,unsigned short b_color,unsigned short a_color)
{            
    FontTypeStruct Font;  
    unsigned int Addr;
	unsigned char i,w;
    Font=FontGetType(f);
    Addr=(unsigned int)Font.ELib;
	w=(Font.Witdh+4)/8;//+7是为了照顾宽度为6/12的字符
    Addr+=(Char-' ')*w*Font.High; 
	for(i=0;i<w;i++)
	{
		mraa_lcd_draw_full_list(dev,(unsigned char *)(Addr+i*Font.High),Font.High,X,Y,f_color,b_color,a_color);
		X+=8;
	}
	return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawfont_word(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,const unsigned char *word,unsigned short f_color,unsigned short b_color,unsigned short a_color)
{            
    
    int  k=0,i, offset,utf8word;
    FontTypeStruct Font; 
    unsigned char buffer[32];
    unsigned char buf[3] = "啊";
    Font=FontGetType(f);
    if((word[0]&0xf0)==0xe0)
    {
        utf8word=word[0];utf8word<<=8;utf8word|=word[1];utf8word<<=8;utf8word|=word[2];
    }else{
        utf8word=word[0];utf8word<<=8;utf8word|=word[1];
    }
    buf[0]=0xB0;
    buf[1]=0xA1;
    while(gb2312_utf8_code[k][1])
    {
        if(gb2312_utf8_code[k][1]==utf8word)
        {
            buf[0]=gb2312_utf8_code[k][0]>>8;
            buf[1]=gb2312_utf8_code[k][0]&0xff;
            break;
        }
        k++;
    }
    offset = (94*(unsigned int)(buf[0]-0xa0-1)+(buf[1]-0xa0-1))*32;
    for(i=0;i<32;i++)buffer[i]=dev->f16p[offset+i];
    mraa_lcd_draw_full_lists(dev,(unsigned char *)(&buffer[0]),16,Font.High,X,Y,f_color,b_color,a_color);
	return MRAA_SUCCESS;
}

mraa_result_t
mraa_lcd_drawfont_string(mraa_lcd_context dev,unsigned short f,unsigned short X,unsigned short Y,const unsigned char *Str,unsigned short f_color,unsigned short b_color,unsigned short a_color)
{            
    unsigned short XX,i;
    FontTypeStruct Font;  
    int offset=0;
	unsigned char *P=(unsigned char *)Str,w;
    Font=FontGetType(f);
    XX=X;
	w=(Font.Witdh+4)/8;//+7是为了照顾宽度为6/12的字符
    int l=strlen(P);
    for(i=0;i<l;i)
    {
        if(P[i]<0x80)
        {
            if((dev->xres-X)<Font.Witdh)
            {
                X=XX;Y+=Font.High;
            } 
            if(P[i]>=0x20&&P[i]<=0x7e)mraa_lcd_drawfont_ascii(dev,f,X,Y,P[i],f_color,b_color,a_color);
            if(P[i]=='\n')
            {
                X=XX;Y+=Font.High;
            }else X+=Font.Witdh;
            i++;
            
        }
        else 
        {
            if((dev->xres-X)<(8*2))
            {
                X=XX;Y+=Font.High;
            } 
            offset=0;
            if(Font.High>16)offset=(((int)Font.High-16)/2+2);
            mraa_lcd_drawfont_word(dev,1616,X,(int)Y+offset,&P[i],f_color,b_color,a_color);
            if((P[i]&0xf0)==0xe0)i+=3;
             else i+=2;
            X+=8*2;
        }
    }
	return MRAA_SUCCESS;
}
int
mraa_lcd_read(mraa_lcd_context dev, char* buf, size_t len)
{
   return 0;
}

int
mraa_lcd_write(mraa_lcd_context dev, const char* buf, size_t len)
{
    char cmd[50]="";
    strcat(cmd,"echo -e '");
    strcat(cmd,buf);
    strcat(cmd,"' > /dev/tty0");
    system(cmd);
    return len;
}

unsigned char * mraa_lcd_getjpg(mraa_lcd_context dev,char * filename, int *w, int *h)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE           *infile;
	unsigned char  *buffer;
	unsigned char *temp;
	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "open %s failed/n", filename);
		exit(-1);
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	*w = cinfo.output_width;
	*h = cinfo.output_height;
	if ((cinfo.output_width > dev->xres) ||(cinfo.output_height > dev->yres)) {
	}
	buffer = (unsigned char *) malloc(cinfo.output_width *cinfo.output_components * cinfo.output_height);
	temp = buffer;
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &buffer, 1);
		buffer += cinfo.output_width * cinfo.output_components;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
    fclose(infile);
	return temp;
	
}

mraa_result_t
mraa_lcd_drawjpg(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name)
{
    int w ,h,i,j;
    int color;
	unsigned char *imgbuf;
	imgbuf = mraa_lcd_getjpg(dev,(char *)name,&w,&h);
	for(j = 0; j < h; j++)
	{
		for( i = 0; i < w; i++)
		{
			color= imgbuf[i*3 + j*w*3];
            color<<=8;
			color|=  imgbuf[i*3 + j*w*3 + 1];
            color<<=8;
			color|= imgbuf[i*3 + j*w*3+2];
            mraa_lcd_drawdot(dev,x+i,y+j,RGB8882RGB565(color));
		}
	}
    free(imgbuf);
    return MRAA_SUCCESS;
}
#define RGB565_MASK_RED        0xF800  
#define RGB565_MASK_GREEN    0x07E0  
#define RGB565_MASK_BLUE       0x001F  
mraa_result_t
mraa_lcd_drawpng(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name)
{
    int i,j;
    int color;
    unsigned char alpha;
    unsigned short tc;
    FILE* file = fopen(name, "rb");
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, file);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    int m_width = png_get_image_width(png_ptr, info_ptr);
    int m_height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int size = m_height * m_width * 4;
    int pos = 0;
    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
    for(i = 0; i < m_height; i++)
    {
        for(j = 0; j < (4 * m_width); j += 4)
        {
            tc=mraa_lcd_getdot(dev,j/4+x,i+y);
            alpha=row_pointers[i][j + 3];
            color= (row_pointers[i][j]*alpha/255)+((tc&RGB565_MASK_RED)>>8)*(255-alpha)/255;
            color<<=8;
			color|=(row_pointers[i][j+1]*alpha/255)+((tc&RGB565_MASK_GREEN)>>3)*(255-alpha)/255;;
            color<<=8;
			color|=(row_pointers[i][j+2]*alpha/255)+((tc&RGB565_MASK_BLUE)<<3)*(255-alpha)/255;
            mraa_lcd_drawdot(dev,j/4+x,i+y,RGB8882RGB565(color));
        }
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(file);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_writeline(mraa_lcd_context dev, const char* buf)
{
    char cmd[50]="";
    strcat(cmd,"echo -e '");
    strcat(cmd,buf);
    strcat(cmd,"' > /dev/tty0");
    system(cmd);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawdotaraay(mraa_lcd_context dev,  uint8_t* data,int length,int cf,int cb)
{
    char w=27,h=27,g=29;
    int x,y,sx=4,sy=5;
    for(x=0;x<8;x++)
    for(y=0;y<8;y++)
    {
        if(data[y]&(1<<x))mraa_lcd_drawrectfill(dev,x*g+sx,y*g+sy,x*g+w+sx,y*g+h+sy,cf);
        else mraa_lcd_drawrectfill(dev,x*g+sx,y*g+sy,x*g+w+sx,y*g+h+sy,cb);
    }
    dev->dot_bcolor=cb;
    dev->dot_fcolor=cf;
    for(x=0;x<8;x++)dev->dotbuf[x]=data[x];
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_selectaraaydot(mraa_lcd_context dev, int x,int y,int color)
{
    char w=27,h=27,g=29;
    int sx=4,sy=5;
    mraa_lcd_drawrect(dev,x*g+sx-1,y*g+sy-1,x*g+w+sx+1,y*g+h+sy+1,color);
    return MRAA_SUCCESS;
}
mraa_result_t
mraa_lcd_drawdotaraaybit(mraa_lcd_context dev,int x,int y,char color)
{
    if(color>0)dev->dotbuf[y]|=(1<<x);
    else dev->dotbuf[y]&=~(1<<x);
    if(dev->dot_fcolor==dev->dot_bcolor)
    {
        dev->dot_fcolor=0xffff;
        dev->dot_bcolor=0x0000;
    }
    return mraa_lcd_drawdotaraay(dev,dev->dotbuf,8,dev->dot_fcolor,dev->dot_bcolor);
    
}
mraa_boolean_t
mraa_lcd_getdotaraaybit(mraa_lcd_context dev,int x,int y)
{
    return dev->dotbuf[y]&(1<<x);
}
mraa_result_t
mraa_lcd_drawdotaraaymove(mraa_lcd_context dev,int x,int y)
{
    #define ROTATE_LEFT(x,  n) ((x) << (n)) | ((x) >> ((8*sizeof(x)) - (n)))
    #define ROTATE_RIGHT(x,  n) ((x) >> (n)) | ((x) << ((8*sizeof(x)) - (n)))
    int i,t;
    unsigned char temp[8];
    if(y>0)
    {
        for(i=0;i<8;i++)temp[i]=dev->dotbuf[i];
        for(i=0;i<8;i++)dev->dotbuf[(y+i)%8]=temp[i];
    }else if(y<0){
        for(i=0;i<8;i++)temp[i]=dev->dotbuf[i];
        for(i=0;i<8;i++)dev->dotbuf[i]=temp[(i-y)%8];
    }
    if(x>0)
    {
        for(i=0;i<8;i++)
        {
            dev->dotbuf[i]=ROTATE_LEFT(dev->dotbuf[i],x);
        }
    }else if(x<0)
    {
        for(i=0;i<8;i++)
        {
            dev->dotbuf[i]=ROTATE_RIGHT(dev->dotbuf[i],0-x);
        }
    }
    if(dev->dot_fcolor==dev->dot_bcolor)
    {
        dev->dot_fcolor=0xffff;
        dev->dot_bcolor=0x0000;
    }
    return mraa_lcd_drawdotaraay(dev,dev->dotbuf,8,dev->dot_fcolor,dev->dot_bcolor);
    
}
mraa_result_t mraa_lcd_screenshot(mraa_lcd_context dev,char * frame, unsigned short width, unsigned short height, FILE * fd, int quality)
{
  JSAMPROW row_ptr[1];
  struct jpeg_compress_struct jpeg;
  struct jpeg_error_mgr jerr;
  char *line, *image;
  int y, x, line_width;
  line =(char *) malloc((width) * 3);
  if (!line)return 0;
  jpeg.err = jpeg_std_error (&jerr);
  jpeg_create_compress (&jpeg);
  jpeg.image_width = width;
  jpeg.image_height= height;
  jpeg.input_components = 3;
  jpeg.in_color_space = JCS_RGB;
  jpeg_set_defaults (&jpeg);
  jpeg_set_quality (&jpeg, quality, TRUE);
  jpeg.dct_method = JDCT_FASTEST;
  jpeg_stdio_dest(&jpeg, fd);
  jpeg_start_compress (&jpeg, TRUE);
  row_ptr[0] = (JSAMPROW) line;
  line_width = width*2;
  image = (char *) frame ;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      line[x*3]=image[x*2+1]&0xf8;
      line[x*3+1]=(image[x*2+1]&0x7)<<5|(image[x*2]&0xe0)>>2;
	  line[x*3+2]=(image[x*2]&0x1f)<<3;
    }
    if (!jpeg_write_scanlines (&jpeg, row_ptr, 1)) {
      jpeg_destroy_compress (&jpeg);
      free(line);
      return 0;
    }
    image += line_width;
  }
  jpeg_finish_compress (&jpeg);
  jpeg_destroy_compress (&jpeg);
  free (line);
  return MRAA_SUCCESS;
}
char * mraa_lcd_screenshotsave(mraa_lcd_context dev)
{
    FILE *infile;
    char *name="/tmp/screenshot.jpg";
    infile = fopen(name, "w");
    mraa_lcd_screenshot(dev,dev->fbp,dev->xres,dev->yres,infile,100);
    fclose(infile);
    return name;
}
mraa_result_t mraa_lcd_screenshotdebug(mraa_lcd_context dev,char * name)
{
    FILE *fp; 
    struct timeval time_now = {0};
    long time_mic = 0;//1微秒 = 1毫秒/1000
    gettimeofday(&time_now,NULL);
    time_mic = time_now.tv_sec*1000*1000 + time_now.tv_usec;
     if((access("/www/tmp",F_OK))==-1)
     {   
        system("ln -s /tmp /www/tmp");
     }
    mraa_lcd_screenshotsave(dev);
    printf("<br><img src='http://%s/tmp/screenshot.jpg?%d'  alt='screenshot' />",name,time_mic);
    return MRAA_SUCCESS;
}
mraa_result_t mraa_lcd_screenprevie(mraa_lcd_context dev,char * name)
{
    FILE *fp; 
    struct timeval time_now = {0};
    long time_mic = 0;//1微秒 = 1毫秒/1000
    gettimeofday(&time_now,NULL);
    time_mic = time_now.tv_sec*1000*1000 + time_now.tv_usec;
     if((access("/www/tmp",F_OK))==-1)
     {   
        system("ln -s /tmp /www/tmp");
     }
    mraa_lcd_screenshotsave(dev);
    printf("%s,%d",name,time_mic);
    return MRAA_SUCCESS;
}



char *mraa_lcd_getfile(char *name,int *l)
{
	FILE*fp;
	char *p; 
	int flen;
	fp=fopen(name,"rb");
	fseek(fp,0L,SEEK_END);   
	flen=ftell(fp);
    *l=flen;	
	p=(char *)malloc(flen+1);  
	if(p==NULL)  
	{  
	fclose(fp);  
	return 0;  
	}  
	fseek(fp,0L,SEEK_SET); 
	fread(p,flen,1,fp);  
	return p;
}

 void *mraa_lcd_client_thread(void *arg)
 {
	 int n;
	 struct timeval timestamp;
	 char *file=0;
	 int length;
	 char buff[MAXLINE];
     fbtreadstream fs;
     memcpy(&fs, arg, sizeof(fbtreadstream));
     mraa_lcd_context dev=(mraa_lcd_context)fs.dev;
     free(arg);
	 n = recv(fs.treadfd,buff,MAXLINE,0);
	 buff[n] = '\0';
   //  printf("recv:%s",buff);
   //  printf("length=%d\n",length);
	 char buffer[10240] = {0};
	 sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            STD_HEADER \
            "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
            "\r\n" \
            "--" BOUNDARY "\r\n");

    if(write(fs.treadfd, buffer, strlen(buffer)) < 0) {
	return;
    }
   int z=0;
   while(dev->stream_run)
   {
    mraa_lcd_screenshotsave(dev);
	file=mraa_lcd_getfile("/tmp/screenshot.jpg",&length);
	gettimeofday(&timestamp,NULL);
	sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                "Content-Length: %d\r\n" \
                "X-Timestamp: %d.%06d\r\n" \
                "\r\n", length, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
    if(write(fs.treadfd, buffer, strlen(buffer))<0)
    {
        close(fs.treadfd);
        free(file);
        return;
    }
	if(write(fs.treadfd, file, length)<0)
    {
        close(fs.treadfd);
        free(file);
        return;
    }
	
    sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
    if(write(fs.treadfd, buffer, strlen(buffer))<0)
    {
        close(fs.treadfd);
        free(file);
        return;
    }
	usleep(100000);
   }
   if(file)free(file);
   close(fs.treadfd);
 }
void *mraa_lcd_server_thread(void *arg)
{
    fbtreadstream fs;
    char buff[MAXLINE];
    int z=10;
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    memcpy(&fs, arg, sizeof(fbtreadstream));
    mraa_lcd_context dev=(mraa_lcd_context)fs.dev;
    free(arg);
	while(dev->stream_run)
	{
		pthread_t client;
        fbtreadstream *fc = malloc(sizeof(fbtreadstream));
        fc->dev=dev;
		if((fc->treadfd = accept(fs.treadfd,(struct sockaddr *)&client_addr, &addr_len))==-1)
		{
            free(fc);
			continue;
		}
		pthread_create(&client, NULL, &mraa_lcd_client_thread,fc);
		pthread_detach(client);
	}
    close(fs.treadfd);
}
mraa_result_t mraa_lcd_screenstream(mraa_lcd_context dev,char * name)
 {
    int on=1;
    
	int listenfd;
	struct sockaddr_in sockaddr;
	memset(&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(15000);
	listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 	if(listenfd<0)return 0;
   	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) 
	{
            perror("setsockopt(SO_REUSEADDR) failed");
    }
	bind(listenfd,(struct sockaddr *) &sockaddr,sizeof(sockaddr));
	listen(listenfd,1024);
    fbtreadstream *fs = malloc(sizeof(fbtreadstream));
    fs->dev=dev;
    fs->treadfd=listenfd;
    dev->stream_run=1;
    pthread_create(&dev->serverthread, NULL, &mraa_lcd_server_thread,fs);
    printf("<br><img src='http://%s:15000'  alt='screenshot' />",name);
	return MRAA_SUCCESS; 
 }
 mraa_result_t
 mraa_lcd_drawpic(mraa_lcd_context dev,unsigned int x,unsigned int y,const unsigned char *name)
{
   if(strstr(name, ".jpg")>0)return mraa_lcd_drawjpg(dev,x,y,name);
   else if(strstr(name, ".png")>0)return mraa_lcd_drawpng(dev,x,y,name);
   else printf("not suppeded");
   return MRAA_SUCCESS; 
}

mraa_result_t 
mraa_lcd_draw_bitmap(mraa_lcd_context dev,int size, FT_Bitmap*  bitmap,FT_Int x,FT_Int y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;
  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= MAPWIDTH || j >= size )
        continue;
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
  return MRAA_SUCCESS; 
}
mraa_result_t 
mraa_lcd_draw_image(mraa_lcd_context dev,int size,int w,int x,int y,unsigned int f,unsigned int b,unsigned int a)
{
  int i,j;
  int color;
  unsigned int tc;
  unsigned char alpha;
  for (  i = 0; i < size; i++ )
  {
    for (  j = 0; j < w; j++ )
    {
        if(a!=b)mraa_lcd_drawdot(dev,x+j,y+i,RGB8882RGB565(b));
        tc=mraa_lcd_getdot(dev,x+j,y+i);
        alpha=image[i][j];
        color= ((f>>16)&0xff*alpha/255)+((tc&RGB565_MASK_RED)>>8)*(255-alpha)/255;
        color<<=8;
        color|=((f>>8)&0xff*alpha/255)+((tc&RGB565_MASK_GREEN)>>3)*(255-alpha)/255;;
        color<<=8;
        color|=(f&0xff*alpha/255)+((tc&RGB565_MASK_BLUE)<<3)*(255-alpha)/255;
        mraa_lcd_drawdot(dev,x+j,y+i,RGB8882RGB565(color));
    }
  }
  return MRAA_SUCCESS; 
}

int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic)  
{  
    char b1, b2, b3, b4, b5, b6;  
    *Unic = 0x0; 
    int utfbytes;
    if((pInput[0]&0xf0)==0xe0)utfbytes=3;else utfbytes=2;
    unsigned char *pOutput = (unsigned char *) Unic;  
    switch ( utfbytes )  
    {  
        case 0:  
            *pOutput     = *pInput;  
            utfbytes    += 1;  
            break;  
        case 2:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            if ( (b2 & 0xE0) != 0x80 )  
                return 0;  
            *pOutput     = (b1 << 6) + (b2 & 0x3F);  
            *(pOutput+1) = (b1 >> 2) & 0x07;  
            break;  
        case 3:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b2 << 6) + (b3 & 0x3F);  
            *(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
            break;  
        case 4:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b3 << 6) + (b4 & 0x3F);  
            *(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
            *(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
            break;  
        case 5:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b4 << 6) + (b5 & 0x3F);  
            *(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
            *(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
            *(pOutput+3) = (b1 << 6);  
            break;  
        case 6:  
            b1 = *pInput;  
            b2 = *(pInput + 1);  
            b3 = *(pInput + 2);  
            b4 = *(pInput + 3);  
            b5 = *(pInput + 4);  
            b6 = *(pInput + 5);  
            if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
                    || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
                    || ((b6 & 0xC0) != 0x80) )  
                return 0;  
            *pOutput     = (b5 << 6) + (b6 & 0x3F);  
            *(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
            *(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
            *(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
            break;  
        default:  
            return 0;  
            break;  
    }  
  
    return utfbytes;  
}  
wchar_t * utf8_unicode(uint8 *str)
{
  int i=0,n=0;
  uint8 buf[2];
  wchar_t *pwc = (wchar_t *)malloc(255);
  while(str[i])
  {
    if(str[i]<0x80)
    {
      pwc[n++]=str[i];i++;
    }else
    {
      unsigned long aaaa;
      enc_utf8_to_unicode_one(&str[i],&aaaa);
      pwc[n++]=aaaa;
      if((str[i]&0xf0)==0xe0)i+=3;else i+=2;
    }
  }
  pwc[n++]=0;
  return pwc;
}
mraa_result_t 
mraa_lcd_drawfreetype_string(mraa_lcd_context dev,uint16 size,uint16 x,uint16 y,const uint8 *str,int color_f,int color_b,int color_a)
{
  FT_Library    library;
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;             
  FT_Vector     pen;                   
  FT_Error      error;
  int a,b;
  double        angle;
  int           target_height;
  int           n;
  int left=0;
  angle         = (0.0/360 )*3.14159*2;    
  target_height = size;
  for(a=0;a<240;a++)for(b=0;b<320;b++)image[a][b]=0;
  error = FT_Init_FreeType( &library );            
  error = FT_New_Face( library,  "/www/pyly/font/freetype.ttf", 0, &face );
  FT_Set_Pixel_Sizes(face, size, 0);
  slot = face->glyph;
  matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
  pen.x = 0;
  pen.y =20;
  wchar_t *chinese_char =utf8_unicode(str);
  for ( n = 0; chinese_char[n]!=0; n++ )
  {
    FT_Set_Transform( face, &matrix, &pen );
    error = FT_Load_Char( face, chinese_char[n], FT_LOAD_RENDER );
    if(error)printf("font error%d\r\n",n);
    if (error) continue;       
    mraa_lcd_draw_bitmap(dev,size,&slot->bitmap,slot->bitmap_left,target_height-target_height*13/100-slot->bitmap_top );
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
    if(slot->bitmap_left>0)left=slot->bitmap_left;
    else if(n>0)left+=size/2;
  }
  free(chinese_char);
  mraa_lcd_draw_image(dev,size,left+slot->bitmap.width,x,y,color_f,color_b,color_a);
  FT_Done_Face(face);
  FT_Done_FreeType(library);
  return MRAA_SUCCESS; 
}
mraa_result_t 
mraa_lcd_drawawesome_index(mraa_lcd_context dev,uint16 size,uint16 x,uint16 y,int ind,int color_f,int color_b,int color_a)
{
  FT_Library    library;
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;             
  FT_Vector     pen;                   
  FT_Error      error;
  int a,b;
  double        angle;
  int           target_height;
  int           n;
  angle         = (0.0/360 )*3.14159*2;    
  target_height = size;
  for(a=0;a<240;a++)for(b=0;b<320;b++)image[a][b]=0;
  error = FT_Init_FreeType( &library );            
  error = FT_New_Face( library,  "/www/pyly/font/awesome.ttf", 0, &face );
  FT_Set_Pixel_Sizes(face, size, 0);
  slot = face->glyph;
  matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
  pen.x = 0;
  pen.y =20;
  FT_Set_Transform( face, &matrix, &pen );
  error = FT_Load_Char( face, 0xf000+ind, FT_LOAD_RENDER );
  if(error)printf("font error=%d\r\n",ind);
  if (error) continue;       
  mraa_lcd_draw_bitmap(dev,size,&slot->bitmap,slot->bitmap_left,target_height-target_height*13/100-slot->bitmap_top );
  pen.x += slot->advance.x;
  pen.y += slot->advance.y;
  free(chinese_char);
  mraa_lcd_draw_image(dev,size,slot->bitmap_left+slot->bitmap.width,x,y,color_f,color_b,color_a);
  FT_Done_Face(face);
  FT_Done_FreeType(library);
  return MRAA_SUCCESS; 
}
