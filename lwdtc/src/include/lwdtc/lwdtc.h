/**
 * \file            lwdtc.h
 * \brief           LwDTC main file
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
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
 * Version:         v1.0.0
 */
#ifndef LWDTC_HDR_H
#define LWDTC_HDR_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "lwdtc/lwdtc_opt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWDTC Lightweight Date, Time & Cron utility
 * \brief           Lightweight Date, Time & Cron utility
 * \{
 */

/* Minimum and maximum values for each of the fields */
#define LWDTC_SEC_MIN      0   /*!< Minimum value for seconds field */
#define LWDTC_SEC_MAX      59  /*!< Maximum value for seconds field */
#define LWDTC_MIN_MIN      0   /*!< Minimum value for minutes field */
#define LWDTC_MIN_MAX      59  /*!< Maximum value for minutes field */
#define LWDTC_HOUR_MIN     0   /*!< Minimum value for hours field */
#define LWDTC_HOUR_MAX     23  /*!< Maximum value for hours field */
#define LWDTC_MDAY_MIN     1   /*!< Minimum value for day in month field */
#define LWDTC_MDAY_MAX     31  /*!< Maximum value for day in month field */
#define LWDTC_MON_MIN      1   /*!< Minimum value for month field */
#define LWDTC_MON_MAX      12  /*!< Maximum value for month field */
#define LWDTC_WDAY_MIN     0   /*!< Minimum value for week day field (min = Sunday, max = Saturday) */
#define LWDTC_WDAY_MAX     6   /*!< Maximum value for week day field (min = Sunday, max = Saturday) */
#define LWDTC_YEAR_MIN     0   /*!< Minimum value for year field */
#define LWDTC_YEAR_MAX     100 /*!< Maximum value for year field */

/**
 * \brief           Calculate size of statically allocated array
 * \param[in]       x: Array
 * \return          Number of elements
 */
#define LWDTC_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           Result enumeration
 */
typedef enum {
    lwdtcOK = 0x00, /*!< Everything is OK */
    lwdtcERR,       /*!< Generic error */
    lwdtcERRPAR,    /*!< Invalid parameter passed to a function */
    lwdtcERRTOKEN,  /*!< Token value is not valid */
} lwdtcr_t;

/**
 * \brief           Cron context variable with parsed information
 * 
 * It is a bit-field of ones and zeros, indicating a match (or not)
 * for date-time comparison to determine if needs to run (or not) a task
 */
typedef struct {
    uint32_t flags;   /*!< List of all sort of flags for internal use */
    uint8_t sec[8];   /*!< Seconds field. Must support bits from 0 to 59 */
    uint8_t min[8];   /*!< Minutes field. Must support bits from 0 to 59 */
    uint8_t hour[3];  /*!< Hours field. Must support bits from 0 to 23 */
    uint8_t mday[4];  /*!< Day number in a month. Must support bits from 0 to 30 */
    uint8_t mon[2];   /*!< Month field. Must support bits from 0 to 11 */
    uint8_t wday[1];  /*!< Week day. Must support bits from 0 (Sunday) to 6 (Saturday) */
    uint8_t year[13]; /*!< Year from 0 - 100, indicating 2000 - 2100. Must support bits 0 to 100 */
} lwdtc_cron_ctx_t;

lwdtcr_t lwdtc_cron_parse_with_len(lwdtc_cron_ctx_t* ctx, const char* cron_str, size_t cron_str_len);
lwdtcr_t lwdtc_cron_parse(lwdtc_cron_ctx_t* ctx, const char* cron_str);
lwdtcr_t lwdtc_cron_parse_multi(lwdtc_cron_ctx_t* cron_ctx, const char** cron_strs, size_t ctx_len, size_t* fail_index);

lwdtcr_t lwdtc_cron_is_valid_for_time(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx);
lwdtcr_t lwdtc_cron_is_valid_for_time_multi_or(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx,
                                               size_t ctx_len);
lwdtcr_t lwdtc_cron_is_valid_for_time_multi_and(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx,
                                                size_t ctx_len);
lwdtcr_t lwdtc_cron_next(const lwdtc_cron_ctx_t* cron_ctx, time_t curr_time, time_t* new_time);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWDTC_HDR_H */
