#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exploit() {
    printf("Exploit successful! Control hijacked.\n");
}

void welcome(char *name) {
    char buf[12];          
    strcpy(buf, name);     
    printf("Welcome %s\n", buf);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <name>\n", argv[0]);
        exit(1);
    }
    welcome(argv[1]);
    return 0;
}
