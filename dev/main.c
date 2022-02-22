#include <stdio.h>
#include <windows.h>
#include "lwdtc/lwdtc.h"

static void
print_bytes_string(const uint8_t* d, size_t btp) {
    printf("|");
    for (size_t i = 0; i < 13 - btp; ++i) {
        printf("        |");
    }
    while (btp-- > 0) {
        for (size_t i = 0; i < 8; ++i) {
            printf("%d", (int)((d[btp] & (1 << (7 - i))) > 0));
        }
        printf("|");
    }
    printf("\r\n");
}

static void
print_cron_ctx(const lwdtc_cron_ctx_t* ctx) {
    printf("|103   96|95    88|87    80|79    72|71    64|63    56|55    48|47    40|39    32|31    24|23    16|15     8|7      0|\r\n");
    printf("||      |||      |||      |||      |||      |||      |||      |||      |||      |||      |||      |||      |||      ||\r\n");
    print_bytes_string(ctx->sec, sizeof(ctx->sec));
    print_bytes_string(ctx->min, sizeof(ctx->min));
    print_bytes_string(ctx->hour, sizeof(ctx->hour));
    print_bytes_string(ctx->mday, sizeof(ctx->mday));
    print_bytes_string(ctx->mon, sizeof(ctx->mon));
    print_bytes_string(ctx->wday, sizeof(ctx->wday));
    print_bytes_string(ctx->year, sizeof(ctx->year));
}

const char* cron_tokens_list[] = {
    "* * * * * * *",                            /* Token is valid all the time, will fire every day */
    "0 * * * * * *",                            /* Token is valid at beginning of each minute (seconds == 0) */
    "* * * * * 2 *",                            /* Fires every each Tuesday */
    "*/5 * * * * * *",                          /* Fires every 5 seconds */
    "*/5 */5 * * * * *",                        /* Fires each 5 seconds in a minute, every 5 minutes
                                                    (min:sec)
                                                    (00:00, 00:05, 00:10, ...)
                                                    (05:00, 05:05, 05:10, ...)
                                                    (10:00, 10:05, 10:10, ...) */
    "0 0 0 * * 5 * ",                           /* Every Friday at midnight */
    "0 0 */2 * * * *",                          /* Every 2 hours at beginning of the hour */
    "* * */2 * * * *",                          /* Every second of every minute every 2 hours */
    "0 0 0 * * 1-5 *",                          /* At midnight, 00:00 every week between Monday and Friday */
    "15 23 */6 * * * * ",                       /* Every 6 hours at (min:sec) 23:15 (00:23:15, 06:23:15, 12:23:15, ...) */
    "0 0 0 1 * * *",                            /* At 00:00:00 beginning of the month */
    "0 0 0 1 */3 * *",                          /* Every beginning of the quarter, every first day in a month in every 3rd month at 00:00:00 */
    "10 15 20 * 8 6 *",                         /* At 20:15:20 every Saturday in August */
    "10 15 20 8 * 6 *",                         /* At 20:15:20 every Saturday that is also 8th day in Month (both must match, day saturday and date 8th) */
};

/* External examples */
extern int cron_basic(void);
extern int cron_multi(void);
extern int cron_dt_range(void);
extern int cron_calc_range(void);

int
main(void) {
    lwdtc_cron_ctx_t cron_ctx = {0};
    lwdtc_dt_t dt;
    time_t rawtime, rawtime_old = 0;
    struct tm* timeinfo;

    /* Run different examples */
    cron_multi();

    /* Range calculation */
    cron_calc_range();

    /* Run example */
    cron_dt_range();
    
    for (size_t i = 0; i < (sizeof(cron_tokens_list) / sizeof(cron_tokens_list[0])); ++i) {
        lwdtcr_t res;
        printf("Parsing token: %s\r\n", cron_tokens_list[i]);
        if ((res = lwdtc_cron_parse(&cron_ctx, cron_tokens_list[i])) != lwdtcOK) {
            lwdtc_cron_parse(&cron_ctx, cron_tokens_list[i]);
        }
        printf("Result (0 = OK, > 0 = KO): %d\r\n", (int)res);
        print_cron_ctx(&cron_ctx);
        printf("----\r\n");
    }

    lwdtc_cron_parse(&cron_ctx, "*/2 * * * * * *");
    while (1) {
        /* Get current time and react on changes only */
        time(&rawtime);
        if (rawtime != rawtime_old) {
            rawtime_old = rawtime;
            timeinfo = localtime(&rawtime);

            printf("Time: %02d.%02d.%04d %02d:%02d:%02d\r\n",
                (int)timeinfo->tm_mday, (int)timeinfo->tm_mon, (int)timeinfo->tm_year + 1900,
                (int)timeinfo->tm_hour, (int)timeinfo->tm_min, (int)timeinfo->tm_sec
            );

            if (lwdtc_cron_is_valid_for_time(timeinfo, &cron_ctx) == lwdtcOK) {
                /* Execute.. */
                printf("Executing\r\n");
            }
        }
        Sleep(10);
    }
    return 0;
}
