#include <stdio.h>
#include "lwdtc/lwdtc.h"

int
main(void) {
    lwdtc_cron_ctx_t ctx = {0};
    
    printf("Parsing token result: %d\r\n", (int)lwdtc_cron_parse(&ctx, "1 * */15 2-15/3 1,2,3,4 3/2 0-23"));

    return 0;
}
