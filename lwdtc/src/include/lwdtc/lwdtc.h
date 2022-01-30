/**
 * \file            lwdtc.h
 * \brief           LwDTC main file
 */

/*
 * Copyright (c) 2022 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwDTC - Lightweight Date, Time & Cron library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#ifndef LWDTC_HDR_H
#define LWDTC_HDR_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include "lwdtc/lwdtc_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWDTC Lightweight Date, Time & Cron utility
 * \brief           Lightweight Date, Time & Cron utility
 * \{
 */

/**
 * \brief           Result enumeration
 */
typedef enum {
    lwdtcOK = 0x00,                             /*!< Everything is OK */
} lwdtcr_t;

/**
 * \brief           Cron context variable with parsed information
 * 
 * It is a bit-field of ones and zeros, indicating a match (or not)
 * for date-time comparison to determine if needs to run (or not) a task
 */
typedef struct {
    uint8_t sec[8];                             /*!< Seconds field. Must support bits from 0 to 59 */
    uint8_t min[8];                             /*!< Minutes field. Must support bits from 0 to 59 */
    uint8_t hour[3];                            /*!< Hours field. Must support bits from 0 to 23 */
    uint8_t mday[4];                            /*!< Day number in a month. Must support bits from 0 to 30 */
    uint8_t mon[2];                             /*!< Month field. Must support bits from 0 to 11 */
    uint8_t year[12];                           /*!< Year from 0 - 100, indicating 2000 - 2100. Must support bits 0 to 100 */
    uint8_t wday[1];                            /*!< Week day. Must support bits from 0 (Sunday) to 6 (Saturday) */
    uint8_t yday[46];                           /*!< Day in a year, Must support bits from 0 to 365 */
    uint8_t week[7];                            /*!< Week number in a year. Must support bits from 0 to 52 */
} lwdtc_cron_ctx_t;

lwdtcr_t    lwdtc_cron_parse(lwdtc_cron_ctx_t* ctx, const char* cron_str);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWDTC_HDR_H */
