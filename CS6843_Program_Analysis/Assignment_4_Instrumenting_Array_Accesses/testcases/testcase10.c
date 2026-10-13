#include <stdio.h>

#include <stdlib.h>

int test(int arg1, int arg2) {
    int arr[20] = {0}, indexArr[10] = {0}, i = 5, j = 5;
    
    if (arg1 > 100) {
        indexArr[i] = 100;
    } else {
        indexArr[i] = 7;
    }
    i = indexArr[i];
    i = arr[i];
    return 0;
}

int main(int argc, char **argv) {
    printf("%d\n", test(atoi(argv[1]), atoi(argv[2])));
    return 0;
}
