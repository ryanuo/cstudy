#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDialog dialog;
    dialog.setWindowTitle(QStringLiteral("01_hello"));
    dialog.resize(320, 120);

    QVBoxLayout layout(&dialog);
    QLabel label(QStringLiteral("Hello Qt 6! \u7b2c\u4e00\u4e2a\u7ec3\u4e60"), &dialog);
    label.setAlignment(Qt::AlignCenter);
    layout.addWidget(&label);

    return dialog.exec();
}
