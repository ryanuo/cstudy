#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QLabel label(QStringLiteral("Hello Qt 6! 第一个练习"));
    label.resize(280, 60);
    label.show();
    return app.exec();
}