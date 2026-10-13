void test(int arg) {
    int a = 10, b = 100, c = 1000, d = 2147483640;
    a = a + 20;
    b = a + b;
    c += arg;
    d = d + 10;
}

int main() {
    test(10);
    return 0;
}
