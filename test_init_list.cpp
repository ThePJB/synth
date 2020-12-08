#include <stdio.h>

struct foo {
    int a;
    char *b;
    foo(int a, char *b) : a{a}, b{b} {

    }
    void print() {
        printf("%d %s\n", a, b);
    }
};

int main(int argc, char** argv) {
    auto a = foo(6, "spaget");
    a.print();
}