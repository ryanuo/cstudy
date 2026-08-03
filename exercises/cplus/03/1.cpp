/**
 *  基础函数模板：类型自动推导
 */
#include <iostream>
using namespace std;

// template <typename T>
// void swap_(T &a, T &b)
// {
//     T temp = a;
//     a = b;
//     b = temp;
// }

template <typename T,typename A>
T minnums(T a, A b)
{
    return a - b;
}

int main()
{
    cout << minnums('A', 10) << endl;
}