#include <iostream>
#include <memory>
using namespace std;

int main()
{
    int *t = new int(10);
    shared_ptr<int> p1(t);

    std::cout << t << std::endl;
    std::cout << p1.get() << std::endl;
}