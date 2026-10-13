#include <stdio.h>

void test(int arg) {
    int i = 0, j = 10;
    for (i = 0; i < 10; i++) {
        j += 1;
        printf("%d\n", j);
        j -= 1;
        printf("%d\n", j);
    }
}

int main() {
    test(10);
    return 0;
}
