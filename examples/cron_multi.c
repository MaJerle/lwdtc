#include "windows.h"
#include <time.h>
#include <stdio.h>
#include "lwdtc/lwdtc.h"

/* Define all cron strings to execute one task */
static const char* cron_strings[] = {
    "* * * * * 2 *",            /* Task should run every second every Tuesday */
    "0 0 0 * * 5 *",            /* Task should run every Friday at midnight */
};

/* Set array of context objects */
static lwdtc_cron_ctx_t cron_ctxs[LWDTC_ARRAYSIZE(cron_strings)];

int
cron_multi(void) {
    /* Define context for CRON, used to parse data to */
    struct tm* timeinfo;
    time_t rawtime, rawtime_old = 0;
    size_t fail_index;

    /* Parse all cron strings */
    if (lwdtc_cron_parse_multi(cron_ctxs, cron_strings, LWDTC_ARRAYSIZE(cron_ctxs), &fail_index) != lwdtcOK) {
        printf("Failed to parse cron at index %d\r\n", (int)fail_index);
        return 0;
    }
    printf("CRONs parsed and ready to go\r\n");

    while (1) {
        /* Get current time and react on changes only */
        time(&rawtime);

        /* Check if new time has changed versus last read */
        if (rawtime != rawtime_old) {
            rawtime_old = rawtime;
            timeinfo = localtime(&rawtime);

            /* 
             * Check if CRON should execute for all possible cron objects
             * 
             * At least one task should be a pass to execute the task
             */
            if (lwdtc_cron_is_valid_for_time_multi_or(timeinfo, cron_ctxs, LWDTC_ARRAYSIZE(cron_ctxs)) == lwdtcOK) {
                printf("Executing CRON task\r\n");
            }
        }

        /* This is sleep from windows.h lib */
        Sleep(100);
    }
    return 0;
}
