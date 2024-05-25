#include <stdio.h>
#include <windows.h>
#include "lwdtc/lwdtc.h"

typedef struct {
    const char* cron_str;
    const char* next_str[6];
} cron_entry_t;

#define CRON_ENTRY(_cron_str_, _next1_, _next2_, _next3_, _next4_, _next5_, _next6_)                                   \
    {                                                                                                                  \
        .cron_str = (_cron_str_),                                                                                      \
        .next_str = {                                                                                                  \
            (_next1_), (_next2_), (_next3_), (_next4_), (_next5_), (_next6_),                                          \
        },                                                                                                             \
    }

/* Variables as reference */
#define TIME_T_START   1693256990
#define TIME_STR_START "2023-08-28_23:09:50" /* Monday */

/*
 * List of test vectors.
 *
 * Time is referenced to:
 * - time_t:    TIME_T_START
 * - time_str:  TIME_STR_START
 * 
 * Times in the parameters represent when, related to the start time (set above)
 * should the CRON fire next time
 */
static cron_entry_t cron_entries[] = {
    /* Fire every second all the time */
    CRON_ENTRY("* * * * * * *", "2023-08-28_23:09:51", "2023-08-28_23:09:52", "2023-08-28_23:09:53", NULL, NULL, NULL),

    /* Fire every beginning of a minute */
    CRON_ENTRY("0 * * * * * *", "2023-08-28_23:10:00", "2023-08-28_23:11:00", "2023-08-28_23:12:00", NULL, NULL, NULL),

    /* Fire every second on Tuesday */
    CRON_ENTRY("* * * * * 2 *", "2023-08-29_00:00:00", "2023-08-29_00:00:01", "2023-08-29_00:00:02", NULL, NULL, NULL),

    /* Fires every 5 seconds every day */
    CRON_ENTRY("*/5 * * * * * *", "2023-08-28_23:09:55", "2023-08-28_23:10:00", "2023-08-28_23:10:05", NULL, NULL,
               NULL),

    /* Fires each 5 seconds in one minute, repeat this minute every 5 minutes
        (00:00, 00:05, 00:10, ..., 05:00, 05:05, 05:10, ..., 10:00, 10:05, 10:10, ...) */
    CRON_ENTRY("*/5 */5 * * * * *", "2023-08-28_23:10:00", "2023-08-28_23:10:05", "2023-08-28_23:10:10", NULL, NULL,
               NULL),

    /* Fire every Friday at midnight */
    CRON_ENTRY("0 0 0 * * 5 * *", "2023-09-01_00:00:00", "2023-09-08_00:00:00", "2023-09-15_00:00:00", NULL, NULL,
               NULL),

    /* Fire every 2 hours, at the beginning of the hour (x:0:0) */
    CRON_ENTRY("0 0 */2 * * * *", "2023-08-29_00:00:00", "2023-08-29_02:00:00", "2023-08-29_04:00:00", NULL, NULL,
               NULL),

    /* Fires every second in an hour, but every second hour */
    CRON_ENTRY("* * */2 * * * *", "2023-08-29_00:00:00", "2023-08-29_00:00:01", "2023-08-29_00:00:02", NULL, NULL,
               NULL),

    /* Fires at midnight, every week between Monday and Friday */
    CRON_ENTRY("0 0 0 * * 1-5 *", "2023-08-29_00:00:00", "2023-08-30_00:00:00", "2023-08-31_00:00:00", NULL, NULL,
               NULL),

    /* Every 6 hours at (min:sec) 23:15 (00:23:15, 06:23:15, 12:23:15, ...) */
    CRON_ENTRY("15 23 */6 * * * *", "2023-08-29_00:23:15", "2023-08-29_06:23:15", "2023-08-29_12:23:15", NULL, NULL,
               NULL),

    /* At the beginning of the month -> first day in a month */
    CRON_ENTRY("0 0 0 1 * * *", "2023-09-01_00:00:00", "2023-10-01_00:00:00", "2023-11-01_00:00:00", NULL, NULL, NULL),

    /* Every beginning of a quarter -> first day every 3rd month */
    CRON_ENTRY("0 0 0 1 3,6,9,12 * *", "2023-09-01_00:00:00", "2023-12-01_00:00:00", "2024-03-01_00:00:00", NULL, NULL,
               NULL),

    /* At 20:15:10 every Saturday in August */
    CRON_ENTRY("10 15 20 * 8 6 *", "2024-08-03_20:15:10", "2024-08-10_20:15:10", "2024-08-17_20:15:10", NULL, NULL,
               NULL),

    /* At 20:15:10 every Saturday that is also 8th day in Month (both must match, day saturday and date 8th) */
    CRON_ENTRY("10 15 20 8 * 6 *", "2024-06-08_20:15:10", "2025-02-08_20:15:10", "2025-03-08_20:15:10", NULL, NULL,
               NULL),

    /* All seconds in a minute except second 48 */
    CRON_ENTRY("49-47 * * * * * *", "2023-08-28_23:09:51", "2023-08-28_23:09:52", "2023-08-28_23:09:53", NULL, NULL,
               NULL),

    /* Every third second from 49 to 07 (49, 52, 55, 58, 01, 04, 07) */
    CRON_ENTRY("49-07/3 * * * * * *", "2023-08-28_23:09:52", "2023-08-28_23:09:55", "2023-08-28_23:09:58", NULL, NULL,
               NULL),

    /* Every beginning of a minute at start of an hour, every Sunday and Tuesday-Friday */
    CRON_ENTRY("0 0 13 * * 0,2-5 *", "2023-08-29_13:00:00", "2023-08-30_13:00:00", "2023-08-31_13:00:00",
               "2023-09-01_13:00:00", "2023-09-03_13:00:00", "2023-09-05_13:00:00"),
};

/* External examples */
extern int cron_basic(void);
extern int cron_multi(void);
extern int cron_dt_range(void);
extern int cron_calc_range(void);

static const char*
prv_format_time_to_str(struct tm* dt) {
    static char str[64];
    sprintf(str, "%04u-%02u-%02u_%02u:%02u:%02u", (unsigned)(dt->tm_year + 1900), (unsigned)(dt->tm_mon + 1),
            (unsigned)dt->tm_mday, (unsigned)dt->tm_hour, (unsigned)dt->tm_min, (unsigned)dt->tm_sec);
    return str;
}

#include <stdint.h>
#define BIT_SET(map, pos) (map)[(pos) >> 3U] |= (1U << ((pos) & 0x07U))

int
main(void) {
    lwdtc_cron_ctx_t cron_ctx = {0};
    time_t rawtime, rawtime_old = 0;
    struct tm* timeinfo;

    /* Get time and print to user */
    rawtime = TIME_T_START;
    timeinfo = localtime(&rawtime);
    printf("Time: %s, raw: %u\r\n", prv_format_time_to_str(timeinfo), (int)rawtime);

    /* Run through all */
    uint64_t time_start = GetTickCount64();
    for (size_t runindex = 0; runindex < 100; runindex++) {
        for (size_t e_idx = 0; e_idx < (sizeof(cron_entries) / sizeof(cron_entries[0])); ++e_idx) {
            lwdtc_cron_parse(&cron_ctx, cron_entries[e_idx].cron_str);

            /* Run next several time and compare */
            rawtime = TIME_T_START;
            for (size_t i = 0; i < (sizeof(cron_entries[0].next_str) / sizeof(cron_entries[0].next_str[0])); ++i) {
                if (cron_entries[e_idx].next_str[i] != NULL) {
                    size_t len_next_str = strlen(cron_entries[e_idx].next_str[i]);

                    if (len_next_str > 0) {
                        const char* time_next;

                        /* Calculate data */
                        lwdtc_cron_next(&cron_ctx, rawtime, &rawtime);
                        timeinfo = localtime(&rawtime);

                        /* Format text */
                        time_next = prv_format_time_to_str(timeinfo);
                        if (strcmp(time_next, cron_entries[e_idx].next_str[i]) != 0) {
                            printf("Test failed: cron: %s, exp: %s, got: %s\r\n", cron_entries[e_idx].cron_str,
                                   cron_entries[e_idx].next_str[i], time_next);
                            return -1;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
    uint64_t time_end = GetTickCount64();
    printf("Total tick: %llu\r\n\r\n", (unsigned long long)(time_end - time_start));

    return 0;

    while (1) {
        /* Get current time and react on changes only */
        time(&rawtime);
        if (rawtime != rawtime_old) {
            rawtime_old = rawtime;
            timeinfo = localtime(&rawtime);

            printf("Time: %s\r\n", prv_format_time_to_str(timeinfo));

            if (lwdtc_cron_is_valid_for_time(timeinfo, &cron_ctx) == lwdtcOK) {
                /* Execute.. */
                printf("Executing\r\n");
            }
        }
        Sleep(10);
    }
    return 0;
}
