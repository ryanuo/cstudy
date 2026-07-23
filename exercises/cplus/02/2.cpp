// 定义一个描述三角形的类 并将三个边长作为类中的公有成员。提供成员函数 计算周长和面积,并验证.(提示三角形面积公式可参看海伦公式,开平方可使用math库中的sqrt 函数)
#include <iostream>
#include <cmath>
using namespace std;

class Triangle
{
public:
    double a, b, c;
    Triangle(double a, double b, double c) : a(a), b(b), c(c) {}
    bool isValid() const
    {
        return (a + b > c) && (a + c > b) && (b + c > a);
    }
    double perimeter() const { return a + b + c; }
    double area() const
    {
        double s = perimeter() / 2.0;
        return sqrt(s * (s - a) * (s - b) * (s - c));
    }
};

int main()
{
    Triangle tri(3.0, 4.0, 5.0);
    if (tri.isValid())
    {
        cout << "周长: " << tri.perimeter() << endl;
        cout << "面积: " << tri.area() << endl;
    }
    else
    {
        cout << "无法构成三角形！" << endl;
    }
    return 0;
}