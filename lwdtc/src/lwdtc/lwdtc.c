/**
 * \file            lwdtc.c
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
#include <stdint.h>
#include <string.h>
#include "lwdtc/lwdtc.h"

#if defined(LWDTC_DEV)
#include <stdio.h>
#define LWDTC_DEBUG(...) /* printf(__VA_ARGS__) */
#else
#define LWDTC_DEBUG(...)
#endif /* defined(LWDTC_DEV) */

/* Internal defines */
#define ASSERT_WITH_RETURN(c, retval)                                                                                  \
    if (!(c)) {                                                                                                        \
        return retval;                                                                                                 \
    }
#define ASSERT_PARAM(c)       ASSERT_WITH_RETURN(c, lwdtcERRPAR)
#define ASSERT_TOKEN_VALID(c) ASSERT_WITH_RETURN(c, lwdtcERRTOKEN)
#define ASSERT_ACTION(c)      ASSERT_WITH_RETURN(c, lwdtcERR)

#define CHAR_IS_NUM(c)        ((c) >= '0' && (c) <= '9')
#define CHAR_TO_NUM(c)        ((c) - '0')

#define BIT_IS_SET(map, pos)  ((map)[(pos) >> 3U] & (1U << ((pos) & 0x07U)))
#define BIT_SET(map, pos)     (map)[(pos) >> 3U] |= (1U << ((pos) & 0x07U))

/**
 * \brief           Private structure to parse cron input
 */
typedef struct {
    lwdtc_cron_ctx_t* ctx; /*!< Cron parser context pointer */

    /* Token input to be filled at the beginning */
    const char* cron_str; /*!< Original cron string start p, later modified by parser on each token */
    size_t cron_str_len;  /*!< Initial cron string length */

    /* Generated token for particular field */
    const char* new_token; /*!< Start of new parsed token */
    size_t new_token_len;  /*!< Length of new parsed token */
} prv_cron_parser_ctx_t;

/**
 * \brief           Parse a number from a string in decimal format.
 * \param[in]       token: Pointer to token string to parse, that starts with number
 * \param[in]       max_len: Maximum allowed number of characters to parse from string
 * \param[in,out]   index: Output variable to write index offset from start of token.
 *                      Variable must be pre-initialized prior function call,
 *                      as function will only add new value to existing one
 * \param[out]      out_num: Pointer to output number
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
static lwdtcr_t
prv_parse_num(const char* token, size_t max_len, size_t* index, size_t* out_num) {
    size_t cnt = 0;

    ASSERT_TOKEN_VALID(CHAR_IS_NUM(*token));

    /* Parse number in decimal format */
    *out_num = 0;
    while (max_len > 0 && CHAR_IS_NUM(token[cnt])) {
        *out_num = (*out_num) * 10U + CHAR_TO_NUM(token[cnt]);
        ++cnt;
    }
    *index += cnt;
    return lwdtcOK;
}

/**
 * \brief           Get start of next token from a list
 * \param[in,out]   parser: Parser structure with all input data
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
static lwdtcr_t
prv_get_next_token(prv_cron_parser_ctx_t* parser) {
    const char* str = parser->cron_str;
    size_t len = parser->cron_str_len;

    /* Remove all leading spaces from the string */
    for (; len > 0 && str != NULL && *str == ' ' && *str != '\0'; ++str, --len) {}
    if (len == 0 || str == NULL || *str == '\0') {
        return lwdtcERRTOKEN;
    }
    parser->new_token = str; /* Set start of the token */

    /* Search for the end of token */
    for (; len > 0 && str != NULL && *str != ' ' && *str != '\0'; ++str, --len) {}
    /* Get token length */
    parser->new_token_len = str - parser->new_token;

    parser->cron_str = str;     /* Set position where next token is about to start (with leading spaces) */
    parser->cron_str_len = len; /* Set remaining length of the input string */
    return lwdtcOK;
}

/**
 * \brief           Parses string token and sets appropriate bits in the
 *                      cron field bit-map, indicating when particular cron is valid
 * \param[in,out]   parser: Parser structure with all input data
 * \param[in]       bit_map: Byte array to construct bit-map for valid cron
 * \param[in]       val_min: Minimum allowed value user can input
 * \param[in]       val_max: Maximum allowed value user can input
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
static lwdtcr_t
prv_get_and_parse_next_token(prv_cron_parser_ctx_t* parser, uint8_t* bit_map, size_t val_min, size_t val_max) {
    size_t idx = 0, bit_start_pos, bit_end_pos, bit_step;
    uint8_t is_range, is_opposite;

    /* Get next token from string */
    ASSERT_ACTION(prv_get_next_token(parser) == lwdtcOK);

    /*
     * Process token string in a while loop
     *
     * Use do-while loop to seamlessly process comma separations
     */
    do {
        bit_start_pos = 0;
        bit_end_pos = SIZE_MAX;
        bit_step = 1;
        is_range = 0;
        is_opposite = 0;

        ASSERT_ACTION(idx < parser->new_token_len); /* Check token length */

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
         *
         * Examples:
         * - "num": Fixed number, min and max values are the same, step is 1
         * - "min-max": Range value, min and max are defined by numbers, step is 1
         * - "min-max/step": Range value with min and max, step defined by number
         * - "max-min": Range value, from max to the end of possible field and from 0 to the min value
         * - "max-min/step": Range value, from max to the end of possible field and from 0 to the min value, step defined by number
         * - "*": All values between possible minimum and maximum
         * - "* /step": All values between possible minimum and maximum for specific field,
         *          step defined by number
         *          (space between * and / is used to be able to keep it as C comment for this example)
         */

        /* Find start character first */
        if (parser->new_token[idx] == '*') {
            idx++;

            /*
             * Star represents any value between minimum and maximum
             * for specific date & time field.
             *
             * Set start and end positions accordingly
             */
            bit_start_pos = val_min;
            bit_end_pos = val_max;
        } else {
            /*
             * Specific number has been entered by user and represents at least bit minimum position,
             * eventually can even set maximum boundary later (when range is used)
             *
             * Since we don't know yet what is following after the number,
             * parse the number and set start and end bits to the same value
             */
            ASSERT_TOKEN_VALID(prv_parse_num(&parser->new_token[idx], parser->new_token_len - idx, &idx, &bit_start_pos)
                               == lwdtcOK);
            bit_end_pos = bit_start_pos;
        }

        /*
         * Character "-" defines range between min and max
         *
         * At this moment, step is still "1", indicating
         * every value between min and max is to be set
         */
        if (idx < parser->new_token_len && parser->new_token[idx] == '-') {
            ++idx;

            /* Parse second part of range */
            ASSERT_ACTION(idx < parser->new_token_len);
            ASSERT_TOKEN_VALID(prv_parse_num(&parser->new_token[idx], parser->new_token_len - idx, &idx, &bit_end_pos)
                               == lwdtcOK);

            /* Stop bit must be always higher or equal than start bit */
            if (bit_start_pos > bit_end_pos) {
                size_t tmp = bit_end_pos;

                /* Change start and stop values */
                bit_end_pos = bit_start_pos;
                bit_start_pos = tmp;
                /* Mark it as opposite direction */
                is_opposite = 1;
            }
            ASSERT_TOKEN_VALID(bit_end_pos >= bit_start_pos);
            is_range = 1;
        }

        /*
         * Character "/" indicates steps between start and stop bit position
         * 
         * A step_bit must be calculated according to input value and
         * end_position must be set to maximum, but only when range is not used,
         * indicating we want to use full range of available value
         */
        if (idx < parser->new_token_len && parser->new_token[idx] == '/') {
            ++idx;
            ASSERT_TOKEN_VALID(prv_parse_num(&parser->new_token[idx], parser->new_token_len - idx, &idx, &bit_step)
                               == lwdtcOK);

            /*
             * If user did not specify range (min-max) values,
             * then all bits to the end of value are valid.
             *
             * Indicate this by setting end position as being maximum
             */
            if (!is_range) {
                bit_end_pos = SIZE_MAX;
            }
        }

        /* Verify valid user inputs, to make sure min value is lower than max one */
        if (bit_start_pos < val_min) {
            LWDTC_DEBUG("bit_start_pos & is less than minimum: %d/%d\r\n", (int)bit_start_pos, (int)val_min);
            return lwdtcERRTOKEN;
        }
        if (bit_end_pos > val_max) {
            /* Full value indicates complete range, perform manual strip */
            if (bit_end_pos != SIZE_MAX) {
                LWDTC_DEBUG("bit_end_pos is greater than maximum: %d/%d\r\n", (int)bit_end_pos, (int)val_max);
                return lwdtcERRTOKEN;
            }
            bit_end_pos = val_max;
        }

        LWDTC_DEBUG("bit_start_pos: %u, bit_end_pos: %u, bit_step: %u, is_opposite: %u\r\n", (unsigned)bit_start_pos,
                    (unsigned)bit_end_pos, (unsigned)bit_step, (unsigned)is_opposite);

        if (is_opposite) {
            size_t bit;

            /* Set bits in map from stop to the end value and from beginning to start value */
            for (bit = bit_end_pos; bit <= val_max; bit += bit_step) {
                BIT_SET(bit_map, bit);
            }
            /* We start at the multiplier of bit_step value */
            for (bit = bit % bit_step + val_min; bit <= bit_start_pos; bit += bit_step) {
                BIT_SET(bit_map, bit);
            }
        } else {
            /* Set bits in map from start to end */
            for (size_t bit = bit_start_pos; bit <= bit_end_pos; bit += bit_step) {
                BIT_SET(bit_map, bit);
            }
        }

        /* If we are not at the end, character must be comma */
        if (idx == parser->new_token_len) {
            break;
        } else if (parser->new_token[idx] != ',') {
            return lwdtcERRTOKEN;
        }
    } while (parser->new_token[idx++] == ','); /* Could be replaced by (1) */
    return lwdtcOK;
}

/**
 * \brief           Parse string with linux crontab-like syntax,
 *                  optionally enriched according to configured settings
 * \param[in]       ctx: Cron context variable used for storing parsed result
 * \param[in]       cron_str: Input cron string to parse data, using valid cron format recognized by the lib
 * \param[in]       cron_str_len: Length of input cron string,
 *                      not counting potential `NULL` termination character
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse_with_len(lwdtc_cron_ctx_t* ctx, const char* cron_str, size_t cron_str_len) {
    prv_cron_parser_ctx_t parser = {0};
    lwdtcr_t res;

    ASSERT_PARAM(ctx != NULL && cron_str != NULL && cron_str_len > 0);
    LWDTC_MEMSET(ctx, 0x00, sizeof(*ctx));

    /* Setup parser */
    parser.ctx = ctx;
    parser.cron_str = cron_str;
    parser.cron_str_len = cron_str_len;

    /*
     * Start parsing fields in fixed order.
     *
     * Each function shall return \ref lwdtcOK,
     * or parse function will immediately return with error
     */

#define ASSERT_GET_PARSE_TOKEN(func_call)                                                                              \
    do {                                                                                                               \
        res = (func_call);                                                                                             \
        if (res != lwdtcOK) {                                                                                          \
            return res;                                                                                                \
        }                                                                                                              \
    } while (0)

    /* Parse seconds field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->sec, LWDTC_SEC_MIN, LWDTC_SEC_MAX));
    LWDTC_DEBUG("Seconds token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse minutes field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->min, LWDTC_MIN_MIN, LWDTC_MIN_MAX));
    LWDTC_DEBUG("Minutes token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse hours field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->hour, LWDTC_HOUR_MIN, LWDTC_HOUR_MAX));
    LWDTC_DEBUG("Hours token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse day in month field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->mday, LWDTC_MDAY_MIN, LWDTC_MDAY_MAX));
    LWDTC_DEBUG("Mday token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse month field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->mon, LWDTC_MON_MIN, LWDTC_MON_MAX));
    LWDTC_DEBUG("Month token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse day in a week field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->wday, LWDTC_WDAY_MIN, LWDTC_WDAY_MAX));
    LWDTC_DEBUG("Weekday token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);

    /* Parse year field */
    ASSERT_GET_PARSE_TOKEN(prv_get_and_parse_next_token(&parser, ctx->year, LWDTC_YEAR_MIN, LWDTC_YEAR_MAX));
    LWDTC_DEBUG("Year token: len: %d, token: %.*s, rem_len: %d\r\n", (int)parser.new_token_len,
                (int)parser.new_token_len, parser.new_token, (int)parser.cron_str_len);
    return res;
}

/**
 * \brief           Parse string with linux crontab-like syntax,
 *                  optionally enriched according to configured settings
 * \param[in]       ctx: Cron context variable used for storing parsed result
 * \param[in]       cron_str: `NULL` terminated cron string, using valid cron format recognized by the lib
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse(lwdtc_cron_ctx_t* ctx, const char* cron_str) {
    return lwdtc_cron_parse_with_len(ctx, cron_str, strlen(cron_str));
}

/**
 * \brief           Parse multiple CRON strins at the same time.
 *                      It returns immediately on first failed CRON
 * \param[in]       cron_ctx: Cron context variable used for storing parsed result
 * \param[in]       cron_strs: Pointer to array of string pointers with cron strings
 * \param[in]       ctx_len: Number of elements to process
 * \param[out]      fail_index: Optional pointer to output variable to store array index of failed CRON.
 *                      Used only if function doesn't return \ref lwdtcOK,
 *                      otherwise pointer doesn't get modified
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise
 */
lwdtcr_t
lwdtc_cron_parse_multi(lwdtc_cron_ctx_t* cron_ctx, const char** cron_strs, size_t ctx_len, size_t* fail_index) {
    lwdtcr_t res = lwdtcERR;

    ASSERT_PARAM(cron_ctx != NULL && cron_strs != NULL && ctx_len > 0);

    /* Parse all input strings, each to its own cron context structure */
    for (size_t i = 0; i < ctx_len; ++i) {
        res = lwdtc_cron_parse_with_len(&cron_ctx[i], cron_strs[i], strlen(cron_strs[i]));
        if (res != lwdtcOK) {
            if (fail_index != NULL) {
                *fail_index = i;
            }
            break;
        }
    }
    return res;
}

/**
 * \brief           Check if cron is active at specific moment of time,
 *                      provided as parameter
 * \param[in]       tm_time: Current time to check if cron works for it.
 *                      Function assumes values in the structure are within valid boundaries
 *                      and does not perform additional check
 * \param[in]       cron_ctx: Cron context object with valid structure
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_cron_is_valid_for_time(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx) {
    lwdtcr_t res = lwdtcOK;

    ASSERT_PARAM(tm_time != NULL && cron_ctx != NULL);

    /* 
     * Cron is valid only if all values are a pass
     * 
     * This is different from crontab linux command where, for example, cron is valid:
     * - When particular day in month occurs 
     * - or when particular day in week occurs, effectively becoming OR operation.
     * 
     * Our cron is a valid when bitwise AND-ed between all fields is a pass
     */
    if (!BIT_IS_SET(cron_ctx->sec, (uint32_t)tm_time->tm_sec) || !BIT_IS_SET(cron_ctx->min, (uint32_t)tm_time->tm_min)
        || !BIT_IS_SET(cron_ctx->hour, (uint32_t)tm_time->tm_hour)
        || !BIT_IS_SET(cron_ctx->mday, (uint32_t)tm_time->tm_mday)
        || !BIT_IS_SET(cron_ctx->mon, (uint32_t)(tm_time->tm_mon + 1))
        || !BIT_IS_SET(cron_ctx->wday, (uint32_t)tm_time->tm_wday)
        || !BIT_IS_SET(cron_ctx->year, (uint32_t)(tm_time->tm_year - 100))) {
        res = lwdtcERR;
    }
    return res;
}

/**
 * \brief           Get next time of fire for specific cron object
 * 
 * This is a dirty implementation and could be improved in the future.
 * For now, we start with one second after current time, and do the roll
 * over all values until we have a match.
 * 
 * \param           cron_ctx: CRON context object
 * \param           curr_time: Current time, used as reference to get new time
 * \param[out]      new_time: Pointer to new time value
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_cron_next(const lwdtc_cron_ctx_t* cron_ctx, time_t curr_time, time_t* new_time) {
    lwdtcr_t res = lwdtcOK;
    struct tm tm_time;

    ASSERT_PARAM(cron_ctx != NULL);
    ASSERT_PARAM(new_time != NULL);

    /* Go to next second, ignore current actual time */
    ++curr_time;
    LWDTC_CFG_GET_LOCALTIME(&tm_time, &curr_time);
    while (lwdtc_cron_is_valid_for_time(&tm_time, cron_ctx) != lwdtcOK) {
        /* 
         * Does the CRON happen in this hour?
         *
         * We do not jump for more than an hour, to avoid any timezone issues.
         * We assume timezone mismatch is not greather than 30min (1800 seconds)
         */
        if (!BIT_IS_SET(cron_ctx->mday, (uint32_t)tm_time.tm_mday)
            || !BIT_IS_SET(cron_ctx->wday, (uint32_t)tm_time.tm_wday)
            || !BIT_IS_SET(cron_ctx->hour, (uint32_t)tm_time.tm_hour)) {
            curr_time += 1800U - (curr_time % 1800U); /* Increase with the beg of a minute alignment */
            goto before_cont;
        }

        /* Does the CRON happen in this minute? */
        if (!BIT_IS_SET(cron_ctx->min, (uint32_t)tm_time.tm_min)) {
            curr_time += 60U - (curr_time % 60U); /* Go to the beg of next minute */
            goto before_cont;
        }

        /* It happens in this minute, so just increase seconds counter and try again */
        ++curr_time;
        if (++tm_time.tm_sec <= LWDTC_SEC_MAX) {
            continue;
        }
    before_cont:
        LWDTC_CFG_GET_LOCALTIME(&tm_time, &curr_time);
    }
    *new_time = curr_time;
    return res;
}

/**
 * \brief           Check if current time fits to at least one of provided context arrays (OR operation)
 * \param[in]       tm_time: Current time to check if cron works for it.
 *                      Function assumes values in the structure are within valid boundaries
 *                      and does not perform additional check
 * \param[in]       cron_ctx: Pointer to array of cron ctx objects
 * \param[in]       ctx_len: Number of context array length
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_cron_is_valid_for_time_multi_or(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx, size_t ctx_len) {
    lwdtcr_t res = lwdtcERR;

    ASSERT_PARAM(cron_ctx != NULL && tm_time != NULL && ctx_len > 0);

    /* Multi-cron context version of lwdtc_cron_is_valid_for_time */
    while (ctx_len-- > 0) {
        res = lwdtc_cron_is_valid_for_time(tm_time, cron_ctx++);
        if (res == lwdtcOK) {
            break;
        }
    }
    return res;
}

/**
 * \brief           Check if current time fits to all provided cron context arrays (AND operation)
 * \param[in]       tm_time: Current time to check if cron works for it.
 *                      Function assumes values in the structure are within valid boundaries
 *                      and does not perform additional check
 * \param[in]       cron_ctx: Pointer to array of cron ctx objects
 * \param[in]       ctx_len: Number of context array length
 * \return          \ref lwdtcOK on success, member of \ref lwdtcr_t otherwise 
 */
lwdtcr_t
lwdtc_cron_is_valid_for_time_multi_and(const struct tm* tm_time, const lwdtc_cron_ctx_t* cron_ctx, size_t ctx_len) {
    lwdtcr_t res = lwdtcERR;

    ASSERT_PARAM(cron_ctx != NULL && tm_time != NULL && ctx_len > 0);

    /* Multi-cron context version of lwdtc_cron_is_valid_for_time */
    while (ctx_len-- > 0) {
        res = lwdtc_cron_is_valid_for_time(tm_time, cron_ctx++);
        if (res != lwdtcOK) {
            break;
        }
    }
    return res;
}
