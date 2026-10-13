int main() {
    int x, y, z, w, u, v;
    int *a, *b, *c, *d, *e, *f;
    int **p, **q, **r, **s;

    a = &x;
    b = &y;
    c = &z;
    d = &w;
    e = &u;
    f = &v;

    p = &a;
    q = &b;
    r = &c;
    s = &d;

    *p = b;    
    *q = c;    
    *r = d;    
    *s = e;    

    *p = e;    
    *q = f;    
    *r = a;    
    *s = c;    

    p = &c;
    q = &d;
    *p = f;   
    *q = a;    

    r = &a;

    s = &b;

    p = &e;
    q = &f;
    *p = d;   
    *q = b;   

    r = &e;
    s = &f;

    a = c;
    b = d;
    c = e;
    d = f;

    *p = a;
    *q = b;

    return 0;
}
