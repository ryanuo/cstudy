#include <stdio.h>
#include <stdlib.h>

int *test()
{
    int *a = malloc(sizeof(int));
    *a = 10;

    return a;
}

int main()
{
    int *p = test();

    printf("%d\n", *p);

    free(p);
    p = NULL;

    return 0;
}
