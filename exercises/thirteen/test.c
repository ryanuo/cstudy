#include <stdio.h>

int main()
{
    FILE *fp;
    int id;
    float price;
    char name[50];

    // 写入数据
    fp = fopen("product.txt", "w+");
    if (fp == NULL)
    {
        printf("文件打开失败！\n");
        return 1;
    }

    fprintf(fp, "ID: %d, 价格: %.2f, 名称: %s\n", 1001, 5999.00, "苹果手机");

    // 重置文件指针到开始
    rewind(fp);

    // 读取数据
    fscanf(fp, "%d %f %s", &id, &price, name);

    printf("读取到的商品信息：\n");
    printf("ID: %d\n", id);
    printf("价格: %.2f元\n", price);
    printf("名称: %s\n", name);

    fclose(fp);
    return 0;
}