void test(int arg) {
    int i = -20, j = -10;
    if (arg > 0) {
        i += 70;
    }
    j += 10;
    i = i * j;
    arg = arg * j;
}

int main() {
    test(10);
    return 0;
}
