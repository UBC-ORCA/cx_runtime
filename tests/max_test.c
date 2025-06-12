#include <stdio.h>
#include <stdlib.h>

static __attribute__ ((noinline)) int scalar_max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int main() {
    int a = (rand() % 10) - 5;
    int b = (rand() % 10) - 5;
    int c = scalar_max(a, b);
    printf("res: %d\n", c);
    return 0;
}
