#include <stdio.h>

#include <stdlib.h>

int test(int arg1, int arg2) {
    int arr[20] = {0}, i = 10, j = 0;
    switch (arg1) {
        case 5:
            j = arr[i+arg1];
            break;
        case 9:
            j = arr[i+arg1];
            break;
        default:
            break;
    }
    return 0;
}

int main(int argc, char **argv) {
    printf("%d\n", test(atoi(argv[1]), atoi(argv[2])));
    return 0;
}
