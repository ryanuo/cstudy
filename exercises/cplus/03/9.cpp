#include <iostream>
using namespace std;

void change(int* p)
{
    cout << "函数里的p地址: " << &p << endl;
    cout << "p保存的地址: " << p << endl;
}

int main()
{
    int a = 10;

    int* p = &a;

    cout << "main里的p地址: " << &p << endl;
    cout << "p保存的地址: " << p << endl;

    change(p);
}