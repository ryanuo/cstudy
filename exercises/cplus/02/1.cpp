// 定义一个描述矩形的类，计算矩形周长和面积
#include <iostream>
using namespace std;

class Rectangle
{
public:
    Rectangle(double width, double height);
    double getArea();
    double getPerimeter();

private:
    double width;
    double height;
};

Rectangle::Rectangle(double width, double height)
{
    this->width = width;
    this->height = height;
}

double Rectangle::getArea()
{
    return width * height;
}

double Rectangle::getPerimeter()
{
    return 2 * (width + height);
}

int main()
{
    Rectangle r(4, 40);
    cout << "这个矩形的面积为： " << r.getArea() << endl;
    cout << "这个矩形的周长为：" << r.getPerimeter() << endl;
    return 0;
}