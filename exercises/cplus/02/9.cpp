#include <iostream>
using namespace std;

class MyClass
{
public:
    int val;

    // 普通构造
    MyClass(int v = 0) : val(v)
    {
        cout << "[构造] 对象创建, val=" << val << endl;
    }

    MyClass(const MyClass &other) : val(other.val)
    {
        cout << "[拷贝构造] 从 val=" << other.val << " 拷贝" << endl;
    }

    // 析构
    ~MyClass()
    {
        cout << "[析构] 对象销毁, val=" << val << endl;
    }
};

MyClass createObj()
{
    MyClass localObj(100);
    return localObj;
}

int main()
{
    MyClass result = createObj();
    return 0;
}