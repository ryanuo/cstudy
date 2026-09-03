#include <iostream>
using namespace std;


class Foo {
public:
    explicit Foo(int base) : m_base(base) { }
    int operator()(int param) { 
        cout << "base = " << m_base << endl;
        cout << "param = " << param << endl;
        return m_base + param;
     }
private:
    int m_base;
};

int main() {
    int base = 2;
    Foo foo(base); //绑定上下文

    std::cout << "foo(1) = " << foo(1) << std::endl; //3
    std::cout << "foo(2) = " << foo(2) << std::endl; //4
    return 0;
}
