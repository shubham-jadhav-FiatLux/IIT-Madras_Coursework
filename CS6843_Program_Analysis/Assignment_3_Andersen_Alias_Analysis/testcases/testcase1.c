int main()
{
    int x,y;
    int *a ;
    int *b ;
    int **h;

    b = &x;
    a = b;
    b = *h;
    h = a;

    return 0;
}