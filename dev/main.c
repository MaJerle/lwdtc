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

int
main(void) {
    lwdtc_cron_ctx_t ctx = {0};
    
    printf("Parsing token result: %d\r\n", (int)lwdtc_cron_parse(&ctx, "1 * */15 2-15/3 1,2,3,4 3/2 0-23"));
    print_ctx(&ctx);

    return 0;
}
