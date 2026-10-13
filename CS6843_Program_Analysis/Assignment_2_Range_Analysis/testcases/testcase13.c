void test(int arg) {
    int i = 0, j = 0;
    for (i = 20; i > -20; i--) {
        if (arg < 10) {
            j = arg + 1;
        }
    }
}

int main() {
    test(10);
    return 0;
}
