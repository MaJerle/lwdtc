#include "windows.h"
#include <time.h>
#include <stdio.h>
#include "lwdtc/lwdtc.h"

int
cron_basic(void) {
    /* Define context for CRON, used to parse data to */
    lwdtc_cron_ctx_t cron_ctx = {0};
    struct tm* timeinfo;
    time_t rawtime, rawtime_old = 0;

    /* Execute cron to be valid every 2 seconds */
    if (lwdtc_cron_parse(&cron_ctx, "*/2 * * * * * *") != lwdtcOK) {
        printf("Error parsing CRON...\r\n");
        while (1) {}
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

            /* Check if CRON should execute */
            if (lwdtc_cron_is_valid_for_time(timeinfo, &cron_ctx) == lwdtcOK) {
                printf("Executing CRON task\r\n");
            }
        }

        /* This is sleep from windows.h lib */
        Sleep(100);
    }
    return 0;
}
