void test(int arg) {
    int a = 5;
    if (arg >= 3) {
        if (arg < 12) {
            a = arg;
        }
    }
}

int main() {
    test(10);
    return 0;
}
