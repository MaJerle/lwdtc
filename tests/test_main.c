#include <stdio.h>
#include "lwdtc/lwdtc.h"

extern int test_run(void);
extern int test_regression_run(void);

int
main(void) {
    int ret = 0;
    printf("Application running\r\n");
    ret = test_run();
    ret |= test_regression_run();
    printf("Done\r\n");
    return ret;
}