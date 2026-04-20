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

#if defined(__unix__) || defined(__APPLE__)
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#define REGRESSION_HAVE_HANG_GUARD 1
static sigjmp_buf g_hang_jmp;
static void
prv_hang_alarm(int sig) {
    (void)sig;
    siglongjmp(g_hang_jmp, 1);
}
#else
#define REGRESSION_HAVE_HANG_GUARD 0
#endif

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

/*
 * Defect: a cron token with a zero step (e.g. "*\/0") set bit_step=0 and
 * then looped `for (bit = start; bit <= end; bit += bit_step)`, which
 * never advanced. Any untrusted cron string could freeze the caller.
 * Expected behaviour: return lwdtcERRTOKEN.
 *
 * On POSIX we guard the call with SIGALRM; without the fix the parser
 * hangs and the alarm escalates into a test failure instead of a
 * frozen CI. On platforms without alarm() we skip this case.
 */
static void
test_step_zero(void) {
#if REGRESSION_HAVE_HANG_GUARD
    lwdtc_cron_ctx_t ctx = {0};
    void (*prev)(int) = signal(SIGALRM, prv_hang_alarm);

    if (sigsetjmp(g_hang_jmp, 1) == 0) {
        alarm(3);
        lwdtcr_t r = lwdtc_cron_parse(&ctx, "*/0 * * * * * *");
        alarm(0);
        CHECK(r == lwdtcERRTOKEN, "step=0 rejected with lwdtcERRTOKEN");
    } else {
        CHECK(0, "step=0 hung (alarm fired)");
    }

    if (sigsetjmp(g_hang_jmp, 1) == 0) {
        alarm(3);
        lwdtcr_t r = lwdtc_cron_parse(&ctx, "0-5/0 * * * * * *");
        alarm(0);
        CHECK(r == lwdtcERRTOKEN, "range/step=0 rejected with lwdtcERRTOKEN");
    } else {
        CHECK(0, "range/step=0 hung (alarm fired)");
    }

    signal(SIGALRM, prev);
#else
    printf("regression skip: step-zero test (no alarm() on this host)\r\n");
#endif
}

int
test_regression_run(void) {
    g_fails = 0;
    test_step_zero();
    test_null_str();
    return g_fails;
}
