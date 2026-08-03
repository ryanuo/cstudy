// 非类型模板参数：编译期常量
#include <iostream>
using namespace std;

template <typename T, int N>
class ArrayData
{
    T data[N];

public:
    void setData(int index, T value)
    {
        data[index] = value;
    }

    T getData(int index)
    {
        return data[index];
    }
};

int main()
{
    ArrayData<int, 5> a;
    a.setData(0, 10);

    cout << a.getData(0) << endl;
}