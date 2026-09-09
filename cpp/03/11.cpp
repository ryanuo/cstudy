#include <iostream>
using namespace std;

class Person
{
public:
    int show() const
    {
        cout << age;
        return age;
    }

private:
    int age = 10;
};

int main()
{
    const Person p;

    cout << p.show();
    return 0;
}