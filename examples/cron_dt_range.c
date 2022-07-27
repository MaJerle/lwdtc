#include "windows.h"
#include <time.h>
#include <stdio.h>
#include "lwdtc/lwdtc.h"

/*
 * This is example for docs user manual
 *
 * Defines time range:
 * - Starts at Monday at 07:00 morning
 * - Ends on Friday at 19:30 evening
 */
static const char* cron_strings[] = {
    "0 * 7/1 * * 1 *",
    "0 * * * * 2-4 *",
    "0 * 7-19 * * 5 *",
    "0 0-30 19 * * 5 *"
};

/* Define context array for all CRON strings */
static lwdtc_cron_ctx_t cron_ctx[LWDTC_ARRAYSIZE(cron_strings)] = {0};

int
cron_dt_range(void) {
    /* Define context for CRON, used to parse data to */
    struct tm* timeinfo;
    time_t rawtime, rawtime_old = 0;

    /* Parse all CRON strings */
    for (size_t i = 0; i < LWDTC_ARRAYSIZE(cron_strings); ++i) {
        if (lwdtc_cron_parse(&cron_ctx[i], cron_strings[i]) != lwdtcOK) {
            printf("Could not parse CRON: %s\r\n", cron_strings[i]);
            while (1) {}
        }
    }

    while (1) {
        /* Get current time and react on changes only */
        time(&rawtime);

        /* Check if new time has changed versus last read */
        if (rawtime != rawtime_old) {
            rawtime_old = rawtime;
            timeinfo = localtime(&rawtime);

            /* Print time to user */
            printf("Time: %02d.%02d.%04d %02d:%02d:%02d\r\n",
                (int)timeinfo->tm_mday, (int)timeinfo->tm_mon, (int)timeinfo->tm_year + 1900,
                (int)timeinfo->tm_hour, (int)timeinfo->tm_min, (int)timeinfo->tm_sec
            );

            /* Check if current time fits inside CRON-defined time range */
            if (lwdtc_cron_is_valid_for_time_multi_or(timeinfo, cron_ctx, LWDTC_ARRAYSIZE(cron_ctx)) == lwdtcOK) {
                printf("Time is within CRON range\r\n");
            } else {
                printf("Time is NOT within CRON range\r\n");
            }
        }

        /* This is sleep from windows.h lib */
        Sleep(100);
    }
    return 0;
}
