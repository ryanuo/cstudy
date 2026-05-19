#include <stdio.h>

int main()
{
    const int MAX = 100;
    const int TOTAL_PRICES = 100;
    int wen, mu, chu;

    for (wen = 0; wen <= MAX / 5; wen++)
    {
        for (mu = 0; mu <= MAX / 3; mu++)
        {
            chu = MAX - wen - mu;
            // 雏鸡必须是3的倍数，并且总价必须等于100
            if (chu >= 0 && chu % 3 == 0 && TOTAL_PRICES == 5 * wen + 3 * mu + chu / 3)
            {
                printf("文=%d, 母=%d, 出=%d\n", wen, mu, chu);
            }
        }
    }
    return 0;
}