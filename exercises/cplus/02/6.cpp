
#include <iostream>
class Student
{
public:
    int age;

    Student(int age)
    {
        this->age = age;
        std::cout << "普通构造" << std::endl;
    }


    Student(const Student& s)
    {
        age = s.age;
        std::cout << "拷贝构造" << std::endl;
    }
};

Student create()
{
    Student s(20);
    std::cout << &s << std::endl;
    return s;
}

int main()
{
    Student s = create();
    std::cout << &s << std::endl;
}