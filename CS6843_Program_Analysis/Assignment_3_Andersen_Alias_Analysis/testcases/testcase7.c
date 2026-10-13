int *a, *b;


int main()
{
    int x, y;
    a = &x;

    for(int i=0; i<10; ++i)
        b = a;
    
    return 0;
}
