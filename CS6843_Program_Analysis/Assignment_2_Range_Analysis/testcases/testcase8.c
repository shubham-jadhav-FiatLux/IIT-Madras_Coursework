void test(int arg) {
    int i = 20, j = 30;
    if (arg > 0) {
        i += 40;
    }
    j = i + j;
}

int main() {
    test(10);
    return 0;
}
