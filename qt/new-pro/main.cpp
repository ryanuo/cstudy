#include "mainwindow.h"
#include "logindialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog ld;
    MainWindow w;
    if(ld.exec()==LoginDialog::Accepted){
        w.setCurrentUser(ld.username());
        w.show();
    }
    return QApplication::exec();
}
