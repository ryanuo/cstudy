#include <iostream>
#include <list>

using namespace std;

int main()
{
    list<int> nums = {10, 20, 30};

    auto it = nums.begin();

    cout << *it << endl;  // 10

    ++it;

    cout << *it << endl;  // 20

    ++it;

    cout << *it << endl;  // 30
}