/**
 * \file            lwdtc_opt.h
 * \brief           LwDTC options
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
#ifndef LWDTC_OPT_HDR_H
#define LWDTC_OPT_HDR_H

/* Uncomment to ignore user options (or set macro in compiler flags) */
/* #define LWDTC_IGNORE_USER_OPTS */

/* Include application options */
#ifndef LWDTC_IGNORE_USER_OPTS
#include "lwdtc_opts.h"
#endif /* LWDTC_IGNORE_USER_OPTS */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \defgroup        LWDTC_OPT Configuration
 * \brief           Default configuration setup
 * \{
 */

/**
 * \brief           Memory set function
 * 
 * \note            Function footprint is the same as \ref memset
 */
#ifndef LWDTC_MEMSET
#define LWDTC_MEMSET(dst, val, len) memset((dst), (val), (len))
#endif

/**
 * \brief           Get the local time (struct tm) from the time_t pointer type
 * 
 * Default implementation uses localtime but user may use gmtime or even create its own implementation,
 * depending on the target system and overall wishes.
 * 
 * \param[in]       _struct_tm_ptr_: Pointer variable to `struct tm` type.
 *                      Variable is a pointer type and does not store actual time data.
 * \param[in]       _const_time_t_ptr_: Pointer to the `time_t` variable to get time from
 */
#ifndef LWDTC_CFG_GET_LOCALTIME
#define LWDTC_CFG_GET_LOCALTIME(_struct_tm_ptr_, _const_time_t_ptr_)                                                   \
    (void)localtime_s((_struct_tm_ptr_), (_const_time_t_ptr_))
#endif

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWDTC_OPT_HDR_H */
