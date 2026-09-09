#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "========== PROGRAM START ==========";
    qDebug() << "当前工作目录:" << QDir::currentPath();

    QDialog dialog;
    dialog.setWindowTitle(QStringLiteral("01_hello"));
    dialog.resize(320, 120);

    QVBoxLayout layout(&dialog);

    QLabel label(
        QStringLiteral("Hello Qt 6! 第一个练习"),
        &dialog
    );

    label.setAlignment(Qt::AlignCenter);
    layout.addWidget(&label);

    qDebug() << "========== BEFORE FILE ==========";

    QFile dat("test.txt");

    qDebug() << "文件路径:" << dat.fileName();

    if (!dat.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "写入打开失败:" << dat.errorString();
    }
    else
    {
        qDebug() << "文件打开成功";

        dat.write("AB");

        qDebug() << "写入完成";

        dat.close();

        qDebug() << "文件关闭";
    }

    if (!dat.open(QIODevice::ReadOnly))
    {
        qDebug() << "读取打开失败:" << dat.errorString();
    }
    else
    {
        qDebug() << "读取打开成功";

        QByteArray data = dat.readAll();

        qDebug() << "size:" << data.size();
        qDebug() << "data:" << data;
        qDebug() << "text:" << QString::fromUtf8(data);

        dat.close();
    }

    qDebug() << "========== BEFORE EXEC ==========";

    return dialog.exec();
}