int main()
{
    int x,y,z;
    int *a, *b, *c;
    int **p;

    *p = c;
    b = &y;
    b = *p;
    p = &a;
    a = &x;
    *p = c;
    c = p;
    c = &z;

    return 0;
}