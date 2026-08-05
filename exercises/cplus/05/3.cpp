#include <iostream>
#include <memory>

using namespace std;

class B;

class A
{
public:
    A()
    {
        cout << "A 构造" << endl;
    }
    ~A()
    {
        cout << "A 析构" << endl;
    }
    shared_ptr<B> b;
};

class B
{
public:
    B()
    {
        cout << "B 构造" << endl;
    }
    ~B()
    {
        cout << "B 析构" << endl;
    }
    // 不增加引用计数
    weak_ptr<A> a;
};

int main()
{
    {
        shared_ptr<A> a = make_shared<A>();
        shared_ptr<B> b = make_shared<B>();

        cout << "A count:"
             << a.use_count()
             << endl;

        cout << "B count:"
             << b.use_count()
             << endl;

        a->b = b;

        b->a = a;

        cout << "\n建立关系后\n";

        cout << "A count:"
             << a.use_count()
             << endl;

        cout << "B count:"
             << b.use_count()
             << endl;
    }

    cout << "\n离开作用域\n";
}