/**
 * Regression tests for known defects in lwdtc_cron_parse / _with_len.
 *
 * Each test documents the defect it guards against. They will fail
 * against a library version that has not received the corresponding
 * fixes.
 */
#include <stdio.h>
#include <string.h>
#include "lwdtc/lwdtc.h"

static int g_fails = 0;

#define CHECK(expr, desc)                                                                                              \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            printf("REGRESSION FAIL: %s\r\n", (desc));                                                                 \
            ++g_fails;                                                                                                 \
        } else {                                                                                                       \
            printf("regression ok:   %s\r\n", (desc));                                                                 \
        }                                                                                                              \
    } while (0)

/*
 * Defect: lwdtc_cron_parse(ctx, NULL) called strlen(NULL) before the
 * NULL check inside lwdtc_cron_parse_with_len, dereferencing a null
 * pointer. lwdtc_cron_parse_multi had the same shape for each entry.
 * Expected behaviour: return lwdtcERRPAR without crashing.
 */
static void
test_null_str(void) {
    lwdtc_cron_ctx_t ctx = {0};
    lwdtcr_t r = lwdtc_cron_parse(&ctx, (const char*)0);
    CHECK(r == lwdtcERRPAR, "lwdtc_cron_parse rejects NULL string");

    lwdtc_cron_ctx_t ctxs[2] = {{0}, {0}};
    const char* strs[2] = {"* * * * * * *", (const char*)0};
    size_t fail_idx = (size_t)-1;
    r = lwdtc_cron_parse_multi(ctxs, strs, 2, &fail_idx);
    CHECK(r == lwdtcERRPAR, "lwdtc_cron_parse_multi rejects NULL entry");
    CHECK(fail_idx == 1, "lwdtc_cron_parse_multi reports failing index");
}

int
test_regression_run(void) {
    g_fails = 0;
    test_null_str();
    return g_fails;
}
