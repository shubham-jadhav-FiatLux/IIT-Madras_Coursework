#include <stdio.h>

#include <stdlib.h>

int test(int arg1, int arg2) {
    int arr[10] = {0}, i = 0, j = 0;
    if (arg1 >= 0) {
        j = arr[arg1];
    }
    return 0;
}

int main(int argc, char **argv) {
    printf("%d\n", test(atoi(argv[1]), atoi(argv[2])));
    return 0;
}
