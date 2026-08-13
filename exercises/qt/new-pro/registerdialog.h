#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    QString username() const;   // 注册成功后返回新用户名

private slots:
    void on_registerButton_clicked();   // 注册
    void on_cancelButton_clicked();     // 取消

private:
    Ui::RegisterDialog *ui;
    QString m_username;
};

#endif // REGISTERDIALOG_H
