#include "noise.h"
#include "log.h"
#include <stdio.h>

enum loglevel loglevel;

coroutine void test_noise() {
    int x = 0;
    nz_real y = 0;

    const struct nz_enum x_enums[] = {
        { 0, "off"},
        { 1, "on"},
        { 2, "super"},
        { 0, 0 }
    };
    nz_param_enum("x", x_enums, &x);
    nz_param_real("y", 0, 1, &y);

    while (1) {
        msleep(now() + 1000);
        fprintf(stderr, "\rx=%d y=%f", x, y);
    }
}

int main(void) {

    go(nz_param_ui());
    //go(test_noise());
    test_noise();

}
