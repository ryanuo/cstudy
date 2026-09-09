#include <iostream>
using namespace std;

class Person
{
public:
    Person(const char *name) : _name(name) {
        cout << "Person " << _name << " is created." << endl;
    }
    string _name;
};

class Student : virtual public Person
{
public:
    Student(const char *name, int num)
        : Person(name), _num(num) {
            cout << "Student " << _name << " is created." << endl;
        }

protected:
    int _num;
};

class Teacher : virtual public Person
{
public:
    Teacher(const char *name, int id)
        : Person(name), _id(id) {
            cout << "Teacher " << _name << " is created." << endl;
        }

protected:
    int _id;
};

class Assistant : public Student, public Teacher
{
public:
    Assistant(const char *name1, const char *name2, const char *name3)
        : Person(name3), Student(name1, 1), Teacher(name2, 2) {
            cout << "Assistant " << _name << " is created." << endl;
        }
};

int main()
{
    Assistant a("张三", "李四", "王五");
    return 0;
}