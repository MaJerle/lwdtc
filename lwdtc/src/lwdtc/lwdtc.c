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

#if defined(LWDTC_DEV)
#include <stdio.h>
#define LWDTC_DEBUG                         printf
#else
#define LWDTC_DEBUG
#endif /* defined(LWDTC_DEV) */

/* Internal defines */
#define ASSERT_PARAM(c)                     if (!(c)) { return lwdtcERRPAR; }
#define ASSERT_TOKEN_VALID(c)               if (!(c)) { return lwdtcERRTOKEN; }
#define ASSERT_ACTION(c)                    if (!(c)) { return lwdtcERR; }
#define CHAR_IS_NUM(c)                      ((c) >= '0' && (c) <= '9')
#define CHAR_TO_NUM(c)                      ((c) - '0')

/**
 * \brief           Private structure to generate next token
 */
typedef struct {
    const char* token_start_orig;               /*!< Very original pointer where token starts */
    size_t token_orig_len;                      /*!< Length of original token */

    const char* token_next_start;               /*!< Pointer where next calculation must start to get new token */
} prv_token_parse_t;

/**
 * \brief           Parse a number from a string
 * \param[in]       token: Pointer to token string to parse, that starts with number
 * \param[in,out]   index: Output variable to write index offset from start of token.
 *                      Variable must be pre-initialized prior function call,
 *                      as function will only add new value to existing one
 * \param[out]      out_num: Pointer to output number
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
static lwdtcr_t
prv_parse_num(const char* token, size_t* index, size_t* out_num) {
    size_t cnt = 0;

    ASSERT_TOKEN_VALID(CHAR_IS_NUM(*token));

    /* Parse number in decimal format */
    *out_num = 0;
    while (CHAR_IS_NUM(token[cnt])) {
        *out_num = (*out_num) * 10 + CHAR_TO_NUM(token[cnt]);
        ++cnt;
    }
    *index += cnt;
    return lwdtcOK;
}

/**
 * \brief           Get start of next token from a list
 * \param[in]       tkn: Pointer to pointer to token.
 *                      Pointer gets modified to the end of output token.
 *                      User can use it to compare if token ended
 * \param[in]       tkn_len: Pointer to input string length.
 *                      It gets modified at each call and adjusts to remaining token length
 * 
 * \param[out]      token_start: Pointer to pointer to variable where
 *                      to point at start of token 
 * \param[out]      len_out: Pointer to optional output variable for token length calculation
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
prv_get_next_token(const char** tkn, size_t* tkn_len, const char** token_start, size_t* len_out) {
    const char* s = *tkn;
    size_t len = *tkn_len;

    /* Remove all leading spaces */
    for (; len > 0 && s != NULL && *s == ' ' && *s != '\0'; ++s, --len) {}
    if (len == 0 || s == NULL || *s == '\0') {
        return lwdtcERRTOKEN;
    }
    *token_start = s;                           /* Set start of the token */

    /* Search for the end of token */
    for (; len > 0 && s != NULL && *s != ' ' && *s != '\0'; ++s, --len);
    *tkn = s;                                   /* Set new token position */
    *len_out = s - *token_start;                /* Get token length */
    *tkn_len = len;                             /* Set remaining length */
    return lwdtcOK;
}

/**
 * \brief           Parses string token and sets appropriate bits in the
 *                      cron field bit-map, indicating when particular cron is valid
 * \param           bit_map: Byte array to construct bit-map for valid cron
 * \param           token: String token to parse
 * \param           token_len: Length of token
 * \param           val_min: Minimum allowed value user can input
 * \param           val_max: Maximum allowed value user can input
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
static lwdtcr_t
prv_parse_token(uint8_t* bit_map, const char* token, const size_t token_len, size_t val_min, size_t val_max) {
    size_t i = 0, bit_start_pos, bit_end_pos, bit_step;
    uint8_t is_range;

    /*
     * Process token string
     *
     * Use do-while loop to seamlessly process "," separations
     */
    do {
        bit_start_pos = 0;
        bit_end_pos = (size_t)-1;
        bit_step = 1;
        is_range = 0;

        ASSERT_ACTION(i < token_len);           /* Check token length */

        /*
         * Token starts with one of 2 possible values:
         *
         * - "*", indicating all values in the field available range
         * - digit, indicating fixed bit position
         * 
         * Followed by first character, second step is optional:
         * 
         * - "/" to indicate steps (applicable for "*" or digit). Can also appear after range step
         * - "-" to indicate range
         * - "," to indicate custom values
         */

        /* Find start character first */
        if (token[i] == '*') {
            i++;
            bit_start_pos = val_min;
        } else {
            ASSERT_TOKEN_VALID(prv_parse_num(&token[i], &i, &bit_start_pos) == lwdtcOK);
            bit_end_pos = bit_start_pos;
        }

        /*
         * Character "-" defines range between min and max
         *
         * At this moment, step is still "1", indicating
         * every value between min and max included
         */
        if (i < token_len && token[i] == '-') {
            ++i;

            /* Parse second part of range */
            ASSERT_ACTION(i < token_len);
            ASSERT_TOKEN_VALID(prv_parse_num(&token[i], &i, &bit_end_pos) == lwdtcOK);

            /* Stop bit must be always higher or equal than start bit */
            ASSERT_TOKEN_VALID(bit_end_pos >= bit_start_pos);
            is_range = 1;
        }

        /*
         * Character "/" indicates steps between start and stop bit position
         * 
         * A step_bit must be calculated & end_position set to maximum,
         * indicating we want to use full range of available value
         */
        if (i < token_len && token[i] == '/') {
            ++i;
            ASSERT_TOKEN_VALID(prv_parse_num(&token[i], &i, &bit_step) == lwdtcOK);
            
            /* When range is not defined, set maximum manually to the top */
            if (!is_range) {
                bit_end_pos = (size_t)-1;
            }
        }

        /* Adapt boundaries */
        if (bit_start_pos < val_min) {
            LWDTC_DEBUG("bit_start_pos & is less than minimum: %d/%d\r\n", (int)bit_start_pos, (int)val_min);
            return lwdtcERRTOKEN;
        }
        if (bit_end_pos > val_max) {
            /* Full value indicates complete range, perform manual strip */
            if (bit_end_pos != (size_t)-1) {
                LWDTC_DEBUG("bit_end_pos is greater than maximum: %d/%d\r\n", (int)bit_end_pos, (int)val_max);
                return lwdtcERRTOKEN;
            }
            bit_end_pos = val_max;
        }

        LWDTC_DEBUG("bit_start_pos: %u, bit_end_pos: %u, bit_step: %u\r\n",
                    (unsigned)bit_start_pos, (unsigned)bit_end_pos, (unsigned)bit_step);

        /* Set bits in map */
        for (size_t bit = bit_start_pos; bit <= bit_end_pos; bit += bit_step) {
            bit_map[bit >> 3] |= 1 << (bit & 0x07);
        }

        /* If we are not at the end, character must be comma */
        if (i == token_len) {
            break;
        } else if (token[i] != ',') {
            return lwdtcERRTOKEN;
        }
    } while (token[i++] == ',');    /* Could be replaced by (1) */
    return lwdtcOK;
}

/**
 * \brief           Parses string with linux crontab-like syntax,
 *                  optionally enriched according to configured settings
 * \param           ctx: Cron context variable used for storing parsed result
 * \param           cron_str: Input cron string to parse data, using valid cron format recognized by the lib
 * \param           cron_str_len: Length of input cron string,
 *                      not counting potential `NULL` termination character
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse_with_len(lwdtc_cron_ctx_t* ctx, const char* cron_str, size_t cron_str_len) {
    size_t i = 0, index = 0, tkn_len, new_token_len;
    const char* tkn, *new_token;

    /* Verify all parameters */
    ASSERT_PARAM(ctx != NULL);
    ASSERT_PARAM(cron_str != NULL);
    ASSERT_PARAM(cron_str_len > 0);

    memset(ctx, 0x00, sizeof(*ctx));            /* Reset structure */

    /* Set input data */
    tkn = cron_str;
    tkn_len = cron_str_len;

    /* Todo: Go to structure and call parsing with structure parameter */
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Seconds token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->sec, new_token, new_token_len, LWDTC_SEC_MIN, LWDTC_SEC_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Minutes token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->min, new_token, new_token_len, LWDTC_MIN_MIN, LWDTC_MIN_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Hours token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->hour, new_token, new_token_len, LWDTC_HOUR_MIN, LWDTC_HOUR_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Mday token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->mday, new_token, new_token_len, LWDTC_MDAY_MIN, LWDTC_MDAY_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Month token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->mon, new_token, new_token_len, LWDTC_MON_MIN, LWDTC_MON_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Weekday token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->wday, new_token, new_token_len, LWDTC_WDAY_MIN, LWDTC_WDAY_MAX) == lwdtcOK);
    
    ASSERT_ACTION(prv_get_next_token(&tkn, &tkn_len, &new_token, &new_token_len) == lwdtcOK);
    LWDTC_DEBUG("Year token: len: %d, token: %.*s, rem_len: %d\r\n", (int)new_token_len, (int)new_token_len, new_token, (int)tkn_len);
    ASSERT_ACTION(prv_parse_token(ctx->year, new_token, new_token_len, LWDTC_YEAR_MIN, LWDTC_YEAR_MAX) == lwdtcOK);

    return lwdtcOK;
}

/**
 * \brief           Parses string with linux crontab-like syntax,
 *                  optionally enriched according to configured settings
 * \param           ctx: Cron context variable used for storing parsed result
 * \param           cron_str: `NULL` terminated cron string, using valid cron format recognized by the lib
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse(lwdtc_cron_ctx_t* ctx, const char* cron_str) {
    return lwdtc_cron_parse_with_len(ctx, cron_str, strlen(cron_str));
}

/**
 * \brief           Check if cron is active at specific moment of time,
 *                      provided as parameter
 * \param           cron_ctx: Cron context object with valid structure
 * \param           tm_time: Current time to check if cron works for it.
 *                      Function assumes values in the structure are within valid boudnaries
 *                      and does not perform additional check
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_cron_is_valid_for_time(const lwdtc_cron_ctx_t* cron_ctx, const lwdtc_dt_t* tm_time) {
    ASSERT_PARAM(cron_ctx != NULL);
    ASSERT_PARAM(tm_time != NULL);

    #define BIT_IS_SET(map, pos)            ((map)[(pos) >> 3] & (1 << ((pos) & 0x07)))

    /* 
     * Cron is valid only if all values are a pass
     * 
     * This is different from crontab linux command where, for example, cron is valid:
     * - When particular day in month occurs 
     * - or when particular day in week occurs, OR in between.
     * 
     * This cron must be bitwise AND-ed between all fields instead
     */
    if (!BIT_IS_SET(cron_ctx->sec, tm_time->sec)
        || !BIT_IS_SET(cron_ctx->min, tm_time->min)
        || !BIT_IS_SET(cron_ctx->hour, tm_time->hour)
        || !BIT_IS_SET(cron_ctx->mday, tm_time->mday)
        || !BIT_IS_SET(cron_ctx->mon, tm_time->mon)
        || !BIT_IS_SET(cron_ctx->wday, tm_time->wday)
        || !BIT_IS_SET(cron_ctx->year, tm_time->year)) {
        return lwdtcERR;
    }
    return lwdtcOK;
}

/**
 * \brief           Convert `struct tm` to lwdtc-compatible date&time structure
 * \param           tm_time: Datetime structure from `time.h` header to be converted
 * \param           dt: Datetime structure to write converted data to
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_tm_to_dt(const struct tm* tm_time, lwdtc_dt_t* dt) {
    ASSERT_PARAM(tm_time != NULL);
    ASSERT_PARAM(dt != NULL);

    dt->sec = tm_time->tm_sec;
    dt->min = tm_time->tm_min;
    dt->hour = tm_time->tm_hour;
    dt->mday = tm_time->tm_mday;
    dt->mon = tm_time->tm_mon;
    dt->wday = tm_time->tm_wday;
    dt->year = tm_time->tm_year - 100;

    return lwdtcOK;
}

/**
 * \brief           Convert lwdtc time structure to `struct tm` data type from `time.h` library
 * \param           dt: Datetime structure to write converted data
 * \param           tm_time: Datetime structure from `time.h` header to be converted
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_dt_to_tm(const lwdtc_dt_t* dt, struct tm* tm_time) {
    ASSERT_PARAM(dt != NULL);
    ASSERT_PARAM(tm_time != NULL);

    tm_time->tm_sec = dt->sec;
    tm_time->tm_min = dt->min;
    tm_time->tm_hour = dt->hour;
    tm_time->tm_mday = dt->mday;
    tm_time->tm_mon = dt->mon;
    tm_time->tm_wday = dt->wday;
    tm_time->tm_year = dt->year + 100;

    return lwdtcOK;
}
