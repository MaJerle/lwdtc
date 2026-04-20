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

int
test_regression_run(void) {
    g_fails = 0;
    /* No tests yet — added in follow-up commits. */
    return g_fails;
}
