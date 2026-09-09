#include <iostream>
using namespace std;

class Base
{
public:
    int a;
    Base()
    {
    }

    virtual void show()
    {
        cout << "Base::show()" << endl;
    }
};

class Derived : public Base
{
public:
    int b;
    Derived()
    {
    }

    void show()
    {
        cout << "Derived::show()" << endl;
    }
};

int main()
{
    Base *p = new Derived;

    p->show();
    p->Base::show();
}