#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long

void f(long x)
{
    long a, b;
    a = x;
    b = 3;
    x = a+b;
    if (a > b)
    {
        WriteLong(a+b);
    }
    else
    {
        f(x);
    }
}

void main()
{
    long a, b, c, d;
    a = 0;
    b = 1;
    c = 2;
    d = c/a;
    if (b == 1)
    {
        a = a+2;
    }
    else
    {
        a = a-2;
    }
    while (c < 4)
    {
        c = c+a*b;
        if (c > d)
        {
            f(b);
        }
    }
    WriteLong(a);
    WriteLong(b);
}
