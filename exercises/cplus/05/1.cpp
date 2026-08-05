#include <memory>
#include <iostream>
using namespace std;

int main()
{
    // unique_ptr<int> p1(new int(10));
    auto p1 = make_unique<int>(10);
    cout << p1 << endl;

    // unique_ptr<int> p2(move(p1)); // 进行所有权的转移
    // int p2 = p1; // 不能将资源进行拷贝
}