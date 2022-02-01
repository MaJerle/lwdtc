/**
 * \file            lwdtc.c
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
#include <string.h>
#include "lwdtc/lwdtc.h"

#define ASSERT_PARAM(c)                     if (!(c)) { return lwdtcERRPAR; }
#define ASSERT_TOKEN_VALID(c)               if (!(c)) { return lwdtcERRTOKEN; }
#define CHAR_IS_NUM(c)                      ((c) >= '0' && (c) <= '9')
#define CHAR_TO_NUM(c)                      ((c) - '0')

#if LWDTC_DEV
#include <stdio.h>
#define LWDTC_DEBUG                         printf
#else
#define LWDTC_DEBUG
#endif /* LWDTC_DEV */

static lwdtcr_t
prv_set_field_bits(lwdtc_cron_ctx_t* ctx, size_t index, size_t bit_start_pos, size_t bit_end_pos, size_t bit_step) {
    size_t bit_min, bit_max;
    uint8_t* bit_map;

    /* Get values for specific index */
    switch (index) {
        case 0: {
            bit_min = LWDTC_SEC_MIN;
            bit_max = LWDTC_SEC_MAX;
            bit_map = ctx->sec;
            break;
        }
        case 1: {
            bit_min = LWDTC_MIN_MIN;
            bit_max = LWDTC_MIN_MAX;
            bit_map = ctx->min;
            break;
        }
        case 2: {
            bit_min = LWDTC_HOUR_MIN;
            bit_max = LWDTC_HOUR_MAX;
            bit_map = ctx->hour;
            break;
        }
        case 3: {
            bit_min = LWDTC_MDAY_MIN;
            bit_max = LWDTC_MDAY_MAX;
            bit_map = ctx->mday;
            break;
        }
        case 4: {
            bit_min = LWDTC_MON_MIN;
            bit_max = LWDTC_MON_MAX;
            bit_map = ctx->mon;
            break;
        }
        case 5: {
            bit_min = LWDTC_YEAR_MIN;
            bit_max = LWDTC_YEAR_MAX;
            bit_map = ctx->year;
            break;
        }
        case 6: {
            bit_min = LWDTC_WDAY_MIN;
            bit_max = LWDTC_WDAY_MAX;
            bit_map = ctx->wday;
            break;
        }
        default: {
            LWDTC_DEBUG("Wrong token index: %d\r\n", (int)index);
            return lwdtcERRTOKEN;
        }
    }

    /* Check lower boundaries */
    if (bit_start_pos < bit_min) {
        LWDTC_DEBUG("bit_start_pos & is less than minimum: %d/%d\r\n", (int)bit_start_pos, (int)bit_min);
        return lwdtcERRTOKEN;
    }
    if (bit_end_pos > bit_max) {
        /* Full value indicates complete range */
        if (bit_end_pos != (size_t)-1) {
            LWDTC_DEBUG("bit_end_pos is greater than maximum: %d/%d\r\n", (int)bit_end_pos, (int)bit_max);
            return lwdtcERRTOKEN;
        }
        bit_end_pos = bit_max;
    }

    /* 
     * Re-adapt bits to actual position in arrays.
     * Decrease by lower boundary both sides
     */
    bit_start_pos -= bit_min;
    bit_end_pos -= bit_min;

    /* Set bits in map */
    for (size_t i = bit_start_pos; i <= bit_end_pos; i += bit_step) {
        bit_map[i / 8] |= 1 << (i & 0x07);
    }
}

/**
 * \brief           Parses string with linux crontab-like syntax,
 *                  optionally enriched according to configured settings
 * \param           ctx: Cron context variable used for storing parsed result
 * \param           cron_str: Input cron string to parse data, string format
 *                  
 *                  `seconds minutes hours day_in_month month year day_in_week`
 * 
 * \param           cron_str_len: Length of input cron string,
 *                      not counting potential `NULL` termination character
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse_with_len(lwdtc_cron_ctx_t* ctx, const char* cron_str, size_t cron_str_len) {
    size_t i = 0, index = 0;

    ASSERT_PARAM(ctx != NULL);
    ASSERT_PARAM(cron_str != NULL);
    ASSERT_PARAM(cron_str_len > 0);

    /* Process complete input string */
    while (1) {
        /* Skip any leading spaces to start of token string */
        for (; i < cron_str_len && cron_str[i] == ' '; ++i) {}

        /* That's more for debug */
        /* TODO: Remove later */
        size_t start_index = i, stop_index;
        for (stop_index = start_index; stop_index < cron_str_len && cron_str[stop_index] != ' '; ++stop_index) {}
        LWDTC_DEBUG("Having token %d: \"%.*s\"\r\n", (int)index, (int)((stop_index - start_index)), &cron_str[start_index]);

        /*
         * Process token string
         *
         * Use do-while loop to seamlessly process "," separations
         */
        do {
            size_t bit_start_pos = 0, bit_end_pos = (size_t)-1, bit_step = 1;

            /*
             * Token starts with either of 2 possible values
             *
             * - "*" indicating all values
             * - digit, indicating fixed bit position
             * 
             * Followed by first character, second step is optional:
             * 
             * - "/" to indicate steps (applicable for "*" or digit)
             * - "-" to indicate range (only applicable for digit)
             * - "," to indicate custom values (only applicable for digit)
             */

            /* Find first character first */
            if (cron_str[i] == '*') {
                i++;
            } else {
                ASSERT_TOKEN_VALID(CHAR_IS_NUM(cron_str[i]));
                while (CHAR_IS_NUM(cron_str[i])) {
                    bit_start_pos = bit_start_pos * 10 + CHAR_TO_NUM(cron_str[i]);
                    ++i;
                }
                bit_end_pos = bit_start_pos;
            }

            /*
             * Character "-" defines range between min and max
             *
             * At this moment, step is still "1", indicating
             * every value between min and max included
             */
            if (cron_str[i] == '-') {
                ++i;
                ASSERT_TOKEN_VALID(CHAR_IS_NUM(cron_str[i]));
                bit_end_pos = 0;
                while (CHAR_IS_NUM(cron_str[i])) {
                    bit_end_pos = bit_end_pos * 10 + CHAR_TO_NUM(cron_str[i]);
                    ++i;
                }

                /* Stop bit must be always higher or equal, but not lower than start */
                ASSERT_TOKEN_VALID(bit_end_pos >= bit_start_pos);
            }

            /*
             * Character "/" indicates steps from start position up to the full available value
             * 
             * A step_bit must be calculated & end_position set to maximum,
             * indicating we want to use full range of available value
             */
            if (cron_str[i] == '/') {
                ++i;
                ASSERT_TOKEN_VALID(CHAR_IS_NUM(cron_str[i]));
                bit_step = 0;
                while (CHAR_IS_NUM(cron_str[i])) {
                    bit_step = bit_step * 10 + CHAR_TO_NUM(cron_str[i]);
                    ++i;
                }
            }
            LWDTC_DEBUG("bit_start_pos: %u, bit_end_pos: %u, bit_step: %u\r\n",
                        (unsigned)bit_start_pos, (unsigned)bit_end_pos, (unsigned)bit_step);

            /* Set bits to the field */
            prv_set_field_bits(ctx, index, bit_start_pos, bit_end_pos, bit_step);

            /* Once we pass this step, char must be either space, comma, or end of string */
            if (cron_str[i] != ' ' && cron_str[i] != ',' && i != (cron_str_len - 1)) {
                return lwdtcERRTOKEN;
            }
        } while (cron_str[i++] == ',');

        /* TODO: Get better end of token check */
        i = stop_index;
        ++index;
        if (i == cron_str_len) {
            break;
        }
    }
    return lwdtcOK;
}

lwdtcr_t
lwdtc_cron_parse(lwdtc_cron_ctx_t* ctx, const char* cron_str) {
    return lwdtc_cron_parse_with_len(ctx, cron_str, strlen(cron_str));
}