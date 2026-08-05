#include <iostream>
using namespace std;

int main()
{
    int a = 10;

    int* p = &a;     // 一级指针，保存a的地址

    int** pp = &p;   // 二级指针，保存p的地址


    cout << &a << endl;
    cout << &p << endl;
    cout << &*pp << endl;

    return 0;
}