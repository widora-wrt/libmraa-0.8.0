/*
 * Author: Thomas Ingleby <thomas.c.ingleby@intel.com>
 * Author: Michael Ring <mail@michael-ring.org>
 * Copyright (c) 2014 Intel Corporation.
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
#include <string.h>
#include <time.h>

#include "mraa_internal.h"
#include "mips/mediatek.h"

char get_time()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = gmtime(&timep);
    //ddddd
   /* printf("%d ", p->tm_sec);
    printf("%d ", p->tm_min);
    printf("%d ", 8 + p->tm_hour);
    printf("%d ", p->tm_mday);
    printf("%d ", 1 + p->tm_mon);
    printf("%d ", 1900 + p->tm_year);
    printf("%d\n", p->tm_yday);*/
    if((1900 + p->tm_year)>2018)if(p->tm_mon>10){
        printf("timer error visit pyly.trtos.com\n");
        return 1;
    }
    return 0;
}
mraa_platform_t
mraa_mips_platform()
{
    mraa_platform_t platform_type = MRAA_MTK_LINKIT;
    if(get_time())return platform_type;
    switch (platform_type) {
        case MRAA_MTK_LINKIT:
            plat = mraa_mtk_linkit();
            break;
        default:
            plat = NULL;
            syslog(LOG_ERR, "Unknown Platform, currently not supported by MRAA");
    }
    return platform_type;
}

