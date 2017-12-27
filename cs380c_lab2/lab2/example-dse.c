#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long

const long N = 100;

void main()
{
    long m[N];
    long mm[N][2];
    long i, j, k;
    i = 0;
    j = 0;
    k = 0;
    while (i < N)
    {
        m[i] = i;
        i = i+1;
    }
    i = 0;
    while (i < N)
    {
        j = m[i]%2;
        if (j == 0)
        {
            mm[i][1] = m[i]+i*2;
        }
        else
        {
            mm[i][0] = m[i]-i%2;
        }
        i = i+1;
        WriteLine();
    }
    i = 0;
    while (i < N)
    {
        j = 0;
        k = i;
        while (i > j)
        {
            if (j%2 == 0)
            {
                k = k+m[j];
            }
            j = j+1;
        }
        i = i+1;
        WriteLong(k);
    }
}
