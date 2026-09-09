#include <stdio.h>

int main()
{
    const double TIER1_LIMIT = 50.0;
    const double TIER2_LIMIT = 200.0;
    const double PRICE_TIER1 = 0.538;
    const double PRICE_TIER2 = 0.568;
    const double PRICE_TIER3 = 0.638;
    double power, price, total_price;

    printf("请输入用电量（千瓦时）：");
    scanf("%lf", &power);

    if (power <= TIER1_LIMIT)
    {
        price = PRICE_TIER1;
        total_price = power * price;
        printf("总电费为：%.2lf元\n", total_price);
    }
    else if (power <= TIER2_LIMIT)
    {
        price = PRICE_TIER2;
        total_price = TIER1_LIMIT * PRICE_TIER1 + (power - TIER1_LIMIT) * price;
        printf("总电费为：%.2lf元\n", total_price);
    }
    else
    {
        price = PRICE_TIER3;
        total_price = TIER1_LIMIT * PRICE_TIER1 + (TIER2_LIMIT - TIER1_LIMIT) * PRICE_TIER2 + (power - TIER2_LIMIT) * price;
        printf("总电费为：%.2lf元\n", total_price);
    }

    return 0;
}