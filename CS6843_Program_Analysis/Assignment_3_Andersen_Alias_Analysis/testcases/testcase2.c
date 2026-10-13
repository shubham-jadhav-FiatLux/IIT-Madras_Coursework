int main()
{
    int x,y;
    int *a ;
    int *b ;
    int *c;
    int **p;

    a = &x;
    b = &y;
    p = &a;
    c = b;
    *p = c;

    return 0;
}