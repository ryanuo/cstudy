#include <iostream>
#include <list>

using namespace std;

int main()
{
    list<int> li;


    // 添加元素
    for(int i = 0; i < 10; i++)
    {
        li.push_back(i * 100);
    }


    cout << "list元素地址：" << endl;

    auto it = li.begin();

    void* prev = nullptr;

    while(it != li.end())
    {
        void* addr = static_cast<void*>(&(*it));

        cout << "value = "
             << *it
             << "  address = "
             << addr;

        // 计算地址差
        if(prev != nullptr)
        {
            long diff =
                (char*)addr - (char*)prev;

            cout << "  diff = "
                 << diff
                 << " bytes";
        }

        cout << endl;


        prev = addr;

        ++it;
    }


    return 0;
}