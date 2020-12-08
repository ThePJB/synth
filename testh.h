#include <stdio.h>

struct astruct {
    int stuff = 0;

    astruct() {};
};

astruct a = astruct();

namespace Test {
    int fooval = 1;
    void foo() {
        printf("spagett %d\n", fooval);
    }
}

int barval = 2;

void bar() {
    printf("espooget %d\n", barval);
}