// 模板特化：为特定类型定制行为
#include <iostream>
using namespace std;

template <typename T>
bool is_equal(T a, T b)
{
    return a == b;
}

template<>
bool is_equal<const char*>(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}

int main()
{
    cout << is_equal(1, 1) << endl;

    cout << is_equal("hello", "hello") << endl;

    std::cout << is_equal("hello", "world") << std::endl;
    return 0;
}