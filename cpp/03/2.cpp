// 基础类模板：通用数据结构
#include <iostream>
using namespace std;

template <typename T>
class Stack
{
    std::vector<T> data;

public:
    void push(T item)
    {
        data.push_back(item);
    }

    T pop()
    {
        T item = data.back();
        data.pop_back();
        return item;
    }

    void print()
    {
        for (auto item : data)
        {
            cout << item << " ";
        }
        cout << endl;
    }
};

int main()
{

    Stack<int> s;

    s.push(1);
    s.push(2);
    s.push(3);

    s.print();
}