#include <iostream>
using namespace std;

void change(int& r)
{
    cout << "函数里的r地址: " << &r << endl;
    cout << "r的值: " << r << endl;
}

int main()
{
    int a = 10;

    int& r = a;

    cout << "main里的r地址: " << &r << endl;
    cout << "r的值: " << r << endl;

    change(r);

    return 0;
}