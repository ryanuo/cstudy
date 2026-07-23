#include <iostream>
#include <string>
using namespace std;

class Employee {
private:
    string name;
    int id;
    string department;
    double salary;
public:
    Employee(string n, int i, string dept, double sal)
        : name(n), id(i), department(dept), salary(sal) {}
    void display() const {
        cout << "姓名: " << name << endl;
        cout << "工号: " << id << endl;
        cout << "部门: " << department << endl;
        cout << "薪资: " << salary << endl;
    }
};

int main() {
    Employee emp("张三", 1001, "技术部", 15000.0);
    emp.display();
    return 0;
}