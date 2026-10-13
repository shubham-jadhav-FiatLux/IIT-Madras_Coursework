#include <stdio.h>

#include <stdlib.h>

int test(int arg1, int arg2) {
    int arr[20] = {0}, i = 0, j = 100;
    while (i < 10) {
        if (j > 20) {
            j = j + arg1;
            j = arr[j];
            return 0;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    printf("%d\n", test(atoi(argv[1]), atoi(argv[2])));
    return 0;
}
