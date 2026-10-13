#include <stdio.h>

void test(int arg) {
    int n = 10, i = 0;
    printf("Enter size of square:\n");
    scanf("%d", &n);
    if (n > 10) {
        printf("Cannot handle n greater than 10\n");
        return;
    } else if (n < 0) {
        printf("Cannot handle negative n\n");
        return;
    }
    i = n;
    if (arg > 3)
        i = i + 10;
}

int main() {
    test(10);
    return 0;
}
