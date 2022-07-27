#include "windows.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "lwdtc/lwdtc.h"

/* Private variables */
static lwdtc_cron_ctx_t* cron_ctxs = NULL;
static size_t cron_ctxs_len = 0;

int
cron_calc_range(void) {
    struct tm start = {
        .tm_year = 2022,
        .tm_mon = 2,
        .tm_mday = 18,
        .tm_wday = 5,
        .tm_hour = 17,
        .tm_min = 3,
        .tm_sec = 0,
    };
    struct tm end = {
        .tm_year = 2022,
        .tm_mon = 2,
        .tm_mday = 23,
        .tm_wday = 3,
        .tm_hour = 16,
        .tm_min = 45,
        .tm_sec = 0,
    };

    /* TODO: Implement algorithm to calculate all necessary cron entries representing time range */
    (void)cron_ctxs;
    (void)cron_ctxs_len;
    (void)start;
    (void)end;
    
    return 1;
}
