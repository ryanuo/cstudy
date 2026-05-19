#include <stdio.h>

int main()
{
    const int total_people = 30; // 修正为30人
    const int total_money = 50;  // 总金额50元
    int x, y, z;                 // x:男人, y:女人, z:小孩
    int count = 0;

    printf("可能的组合有：\n");

    // 男人最多不可能超过 total_money / 3
    for (x = 0; x <= total_money / 3; x++)
    {
        // 女人最多不可能超过 total_money / 2
        for (y = 0; y <= total_money / 2; y++)
        {
            z = total_people - x - y; // 小孩人数由总人数决定

            // 必须保证小孩人数非负，且总金额刚好等于50
            if (z >= 0 && (3 * x + 2 * y + z == total_money))
            {
                printf("方案%d: 男人%d个，女人%d个，小孩%d个\n", ++count, x, y, z);
            }
        }
    }

    return 0;
}