void test(int arg) {
    int a = 10, b = 40, c = -10, d = 40, e = -10, f = -40, g = -214748369, h = 214748369;
    a = a * b;
    c = c * d;
    e = e * f;
    g = g * 10;
    h = h * 10;
}

int main() {
    test(10);
    return 0;
}
