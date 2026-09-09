#include <iostream>
using namespace std;

class A
{

public:
    int x;

    A(int a) : x(a)
    {
    }

    A operator+(const A &b)
    {
        return x + b.x;
    }

    // prefix
    A operator++()
    {
        return A(++x);
    }

    // postfix
    A operator++(int)
    {
        return A(x++);
    }

    A operator+=(const A &b)
    {
        x += b.x;
        return x;
    }

    friend A operator--(A &a, int)
    {
        // 后缀自减逻辑：先保存旧值，再自减，最后返回旧值
        A temp(a.x); // 创建一个临时对象，保存当前的 x (拷贝构造)
        a.x--;       // 修改原对象的值
        return temp; // 返回修改前的旧值
    }

    friend A operator--(A &a)
    {
        --a.x;
        return a;
    }
    int getX() const
    {
        return x;
    }
};

int main()
{
    A a(10);
    A b(20);

    A c = a++;
    cout << c.getX() << endl;

    return 0;
}