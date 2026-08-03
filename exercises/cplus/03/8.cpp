// 可变参数模板：接受任意数量参数
#include <iostream>
using namespace std;

void print()
{
    std::cout << std::endl;
}

template <typename T, typename... Args>
void print(T first, Args... rest)
{
    std::cout << first << " ";
    print(rest...);
}

int main()
{
    print(1, 2.5, "hello", 'A');
}