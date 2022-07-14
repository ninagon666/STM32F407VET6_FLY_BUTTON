/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2018,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		字体文件
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.26
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2018-05-24
 ********************************************************************************************************************/

#ifndef _SEEKFREE_FONT_h
#define _SEEKFREE_FONT_h
#include "stm32f4xx_hal.h"

typedef unsigned char uint8;       //  8 bits
typedef unsigned short int uint16; // 16 bits
typedef unsigned long int uint32;  // 32 bits
typedef unsigned long long uint64; // 64 bits

typedef signed char int8;       //  8 bits
typedef signed short int int16; // 16 bits
typedef long int int32;         // 32 bits
typedef long long int64;        // 64 bits

typedef volatile int8 vint8;   //  8 bits
typedef volatile int16 vint16; // 16 bits
typedef volatile int32 vint32; // 32 bits
typedef volatile int64 vint64; // 64 bits

typedef volatile uint8 vuint8;   //  8 bits
typedef volatile uint16 vuint16; // 16 bits
typedef volatile uint32 vuint32; // 32 bits
typedef volatile uint64 vuint64; // 64 bits

extern const uint8 tft_ascii[95][16];

#endif
