#include <iostream>
using namespace std;

int main()
{
    int a = 10;

    int* p = &a;
    int& r = a;


    cout << sizeof(p) << endl;
    cout << sizeof(r) << endl;

}