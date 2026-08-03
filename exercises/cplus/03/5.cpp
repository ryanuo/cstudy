#include <iostream>
using namespace std;

template <typename T>
T Max(const T a, const T b)
{
    cout << "1" << endl;
    return a > b ? a : b;
}
template <typename T>
T Max(const T a, const T b, const T c)
{
    cout << "2" << endl;
    T t;
    t = Max(a, b);
    return Max(t, c);
}
int Max(const int a, const char b)
{
    cout << "3" << endl;
    return a > b ? a : b;
}

int main()
{
    Max(9.3, 0.5);
    Max(9, 5, 23);
    Max('a', 'b', 'c');
    Max(1, '1');
    Max("lelele", "hahaha");
}