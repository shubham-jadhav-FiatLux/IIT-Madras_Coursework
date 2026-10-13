int main()
{
    int x,y,z;
    int *a, *b, *c;
    int **p;

    *p = c;
    b = &y;
    b = *p;

    if(c==0)
        p = &a;
    else 
        p = &y;

    a = &x;
    *p = c;
    c = p;
    c = &z;

    return 0;
}