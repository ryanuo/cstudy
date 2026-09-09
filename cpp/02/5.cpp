#include <iostream>

class Test
{
    friend void operator1(std::ostream &out, const Test &obj)
    {
        out << "a = " << obj.a << std::endl;
    }

public:
    static int count;
    static Test *ptr;
    int a;

    Test(int val = 0) : a(val)
    {
        std::cout << "Constructor called. a = " << a << std::endl;
        ptr = this;
    }

    static void printInfo()
    {
        std::cout << "Count: " << count << std::endl;

        if (ptr != nullptr)
        {
            std::cout << "Last Object's a: " << ptr->a << std::endl;
        }
        else
        {
            std::cout << "No object created yet." << std::endl;
        }
    }

protected:
    int b;
};

int Test::count = 0;
Test *Test::ptr = nullptr;

int main()
{
    Test t1(10);
    Test::count++;

    Test t2(20);
    Test::count++;

    Test::printInfo();

    return 0;
}