// 定义一个描述学生基本情况的类，数据成员包括姓名、学号、C++成绩、英语和数学成绩,成员函数包括输出数据，求出每个学生的总成绩和平均成绩.可尝试采用C++ 的输入输出流来输出数据
#include <iostream>
#include <string>
using namespace std;

class Student
{
public:
    string name;
    int id;
    int cpp_score;
    int english_score;
    int math_score;
    int total_score;
    double average_score;
    void input_data();
    void print_data();
    void calculate_total_score();
    void calculate_average_score();
};

void Student::print_data()
{
    cout << "--------------------------------------------------" << endl;
    cout << "姓名：" << name << endl;
    cout << "学号：" << id << endl;
    cout << "C++成绩：" << cpp_score << endl;
    cout << "英语成绩：" << english_score << endl;
    cout << "数学成绩：" << math_score << endl;
    cout << "总成绩：" << total_score << endl;
    cout << "平均成绩：" << average_score << endl;
    cout << "--------------------------------------------------" << endl;
}

void Student::calculate_total_score()
{
    total_score = cpp_score + english_score + math_score;
}

void Student::calculate_average_score()
{
    average_score = total_score / 3.0;
}

void Student::input_data()
{
    cout << "请输入学号：";
    cin >> id;
    cout << "请输入姓名：";
    cin >> name;
    cout << "请输入C++成绩：";
    cin >> cpp_score;
    cout << "请输入英语成绩：";
    cin >> english_score;
    cout << "请输入数学成绩：";
    cin >> math_score;
}

int main()
{
    Student student;
    student.input_data();
    student.calculate_total_score();
    student.calculate_average_score();
    student.print_data();
}