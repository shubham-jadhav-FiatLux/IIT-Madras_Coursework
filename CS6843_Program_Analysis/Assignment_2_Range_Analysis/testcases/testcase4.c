void test(int arg) {
    int a = 10, b = 1000, c = -2147483640;
    a = a - 20;
    b = arg - b;
    c = c - 10;
}

int main() {
    test(10);
    return 0;
}
