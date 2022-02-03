#include <stdio.h>
#include "lwdtc/lwdtc.h"

static void
print_bytes_string(const uint8_t* d, size_t btp) {
    while (btp-- > 0) {
        for (size_t i = 0; i < 8; ++i) {
            printf("%d", (int)((d[btp] & (1 << (7 - i))) > 0));
        }
        printf(" ");
    }
    printf("\r\n");
}

static void
print_ctx(const lwdtc_cron_ctx_t* ctx) {
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
    "* * * * * 2 *",                            /* Fires every second each Tuesday */
    "*/5 * * * * * *",                          /* Fires every 5 seconds */
    "*/5 */5 * * * * *",                        /* Fires each 5 seconds in a minute, every 5 minutes
                                                    (min:sec)
                                                    (00:00, 00:05, 00:10, ...)
                                                    (05:00, 05:05, 05:10, ...)
                                                    (10:00, 10:05, 10:10, ...) */
    "0 0 0 * * 5 * ",                           /* Every Friday at midnight */
    "0 0 /2 * * * *",                           /* Every 2 hours at beginning of the hour */
    "* * /2 * * * *",                           /* Every second of every minute every 2 hours */
    "0 0 0 * * 1-5 *",                          /* At midnight, 00:00 every week between Monday and Friday */
    "15 23 */6 * * * * ",                       /* Every 6 hours at (min:sec) 23:15 (00:23:15, 06:23:15, 12:23:15, ...) */
    "0 0 0 1 * * *",                            /* At 00:00:00 beginning of the month */
    "0 0 0 1 */3 * *",                          /* Every beginning of the quarter, every first day in a month in every 3rd month at 00:00:00 */
    "10 15 20 * 8 6 *",                         /* At 20:15:20 every Saturday in August */
    "10 15 20 8 * 6 *",                         /* At 20:15:20 every Saturday that is also 8th day in Month (both must match, day saturday and date 8th) */
};

int
main(void) {
    lwdtc_cron_ctx_t ctx = {0};
    
    for (size_t i = 0; i < (sizeof(cron_tokens_list) / sizeof(cron_tokens_list[0])); ++i) {
        printf("Parsing token: %s\r\n", cron_tokens_list[i]);
        printf("Result (0 = OK, > 0 = KO): %d\r\n", (int)lwdtc_cron_parse(&ctx, cron_tokens_list[i]));
        print_ctx(&ctx);
        printf("----\r\n");
    }
    return 0;
}
