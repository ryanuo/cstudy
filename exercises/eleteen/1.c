#include <stdio.h>

void fun1()
{
    int n = 2341;
    int res = 0;
    int i;

    while (n != 0)
    {
        i = n % 10;
        n = n / 10;
        res += i * i;
    }

    printf("%d\n", res);
}

void fun2()
{
    int n = 10, sum = 0;
    for (int i = 0; i <= n; i++)
    {
        sum += i;
    }

    printf("%d\n", sum);
}

int fun3(int n)
{
    if (n == 1)
        return 1;

    return n + fun3(n - 1);
}

typedef struct _node
{
    int data;
    struct _node *prev;
    struct _node *next;
} node;

void doublelist_insert(node *p, node *q)
{
    p->prev->next = q;
    q->next = p;
    q->prev = p->prev;

    p->prev = q;
}

struct fruit
{
    int no;
    char name[8];
    float price;
};

void fun5()
{
    FILE *fp = NULL;

    if ((fp = fopen("info.dat", "r")) == NULL)
    {
        return;
    }

    struct fruit temp;
    while (fscanf(fp, "%s %d %f", temp.name, &temp.no, &temp.price) == 3)
    {
        printf("%s %d %f\n", temp.name, temp.no, temp.price);
    }
    fclose(fp);
}

char *strdel(char *string, char n)
{
    int j = 0;
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != n)
        {
            if (j != i)
            {
                string[j] = string[i];
            }

            j++;
        }
    }

    string[j] = '\0';

    return string;
}

void fun6()
{
    int a, b, c, d, i, j, k;
    a = 10;
    b = c = d = 5;
    i = j = k = 0;
    for (; a > b; ++b)
        i++;
    while (a > ++c)
        j++;
    do
        k++;
    while (a > d++);
    printf("i=%d, j=%d, k=%d\n", i, j, k);
}

int main(int argc, char **argv)
{
    fun6();

    printf("%d\n", fun3(10));
    return 0;
}