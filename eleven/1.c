#include <stdio.h>
#include <stdlib.h>

struct Product
{
    char id;
    char name[20];
    float price;
    int total;
};

void displayProducts(struct Product);
void displayAllProducts(struct Product[], int);
struct Product findMaxPriceProduct(struct Product[], int);
int main(int argc, char **argv)
{
    // 静态分配
    printf("-------------------静态分配--------------------------\n");
    struct Product p1 = {101, "鼠标", 50.23, 200};
    struct Product p2 = {102, "键盘", 100.99, 300};
    struct Product p3 = {103, "显示器", 500.89, 400};
    struct Product p4 = {104, "CPU", 1000.22, 500};

    struct Product products[4] = {p1, p2, p3, p4};

    displayAllProducts(products, 4);

    printf("最大价格商品：\n");
    displayProducts(findMaxPriceProduct(products, 4));

    // 动态分配 malloc方法
    printf("----------------------------动态分配malloc方法----------------------------\n");
    struct Product *dym_products = (struct Product *)malloc(sizeof(struct Product) * 4);
    if (dym_products == NULL)
    {
        printf("内存分配失败！\n");
        return -1;
    }
    dym_products[0] = p1;
    dym_products[1] = p2;
    dym_products[2] = p3;
    dym_products[3] = p4;

    displayAllProducts(dym_products, 4);
    printf("最大价格商品：\n");
    displayProducts(findMaxPriceProduct(dym_products, 4));
    free(dym_products);

    // 动态分配 calloc方法
    printf("----------------------------动态分配calloc方法----------------------------\n");
    struct Product *dym_call_products = (struct Product *)calloc(4, sizeof(struct Product));
    if (dym_call_products == NULL)
    {
        printf("内存分配失败！\n");
        return -1;
    }
    dym_call_products[0] = p1;
    dym_call_products[1] = p2;
    dym_call_products[2] = p3;
    dym_call_products[3] = p4;

    displayAllProducts(dym_call_products, 4);
    printf("最大价格商品：\n");
    displayProducts(findMaxPriceProduct(dym_call_products, 4));

    free(dym_call_products);

    return 0;
}

void displayProducts(struct Product p)
{
    printf("编号：%d\n", p.id);
    printf("名称：%s\n", p.name);
    printf("价格：%.2f\n", p.price);
    printf("库存：%d\n", p.total);
    printf("---------------------------\n");
}

void displayAllProducts(struct Product products[], int n)
{
    printf("一共有%d个商品\n", n);
    for (int i = 0; i < n; i++)
    {
        printf("第%d个商品\n", i + 1);
        displayProducts(products[i]);
    }
}

struct Product findMaxPriceProduct(struct Product products[], int n)
{
    struct Product max = products[0];
    for (int i = 1; i < n; i++)
    {
        if (products[i].price > max.price)
        {
            max = products[i];
        }
    }
    return max;
}