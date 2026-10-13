int main()
{
    int x,y,z;
    int *a, *b, *c;
    int **p, **q;

    a = &x;
    b = &y;

    q = b;
    *p = a;
    p = &a;
    *p = *q;

    return 0;
}