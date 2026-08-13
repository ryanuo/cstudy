#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString username() const;   // 登录成功后返回用户名

private slots:
    void on_pushButton_clicked();    // 登录
    void on_pushButton_2_clicked();  // 退出
    void on_pushButton_3_clicked();  // 注册

private:
    Ui::LoginDialog *ui;
    QString m_username;
};

#endif // LOGINDIALOG_H
