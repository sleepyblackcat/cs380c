//test control flow hierarchy
#include <stdio.h>
#define WriteLine() printf("\n");
#define WriteLong(x) printf(" %lld", (long)x);
#define ReadLong(a) if (fscanf(stdin, "%lld", &a) != 1) a = 0;
#define long long long



void main()
{
  long a, b;

  a = 0;
  while(a < 10){
      b = 0;
      a = a + 1;
  }
  WriteLong(a);
  WriteLine();
}


/*
 expected output:
 10
*/
