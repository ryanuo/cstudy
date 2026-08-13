#include "logindialog.h"
#include "ui_logindialog.h"
#include <QPixmap>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("用户登录");

    QPixmap pixmap(":/new/prefix1/images/logined.jpg");
    QPixmap scaledPixmap = pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(scaledPixmap);
    ui->label->setAlignment(Qt::AlignCenter);

    setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d9d9d9;
            border-radius: 6px;
            padding: 7px 10px;
            font-size: 13px;
            color: #333333;
            background: #ffffff;
            selection-background-color: #d6e4ff;
            selection-color: #333333;
        }
        QLineEdit:hover {
            border-color: #b3b3b3;
        }
        QLineEdit:focus {
            border-color: #409eff;
        }
    )");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

// 登录：固定账号 admin / 123456（后续可接数据库/后端）
void LoginDialog::on_pushButton_clicked()
{
    QString username = ui->lineEdit_4->text().trimmed();
    QString password = ui->lineEdit_3->text();

    if (username == "admin" && password == "123456") {
        m_username = username;   // 记录登录用户名，供主窗口状态栏显示
        accept();   // exec() 返回 Accepted，main.cpp 据此显示主窗口
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");
        ui->lineEdit_3->clear();
        ui->lineEdit_3->setFocus();
    }
}

QString LoginDialog::username() const
{
    return m_username;
}

// 退出
void LoginDialog::on_pushButton_2_clicked()
{
    reject();
}

void LoginDialog::on_pushButton_3_clicked()
{
    QMessageBox::warning(this, "注册", "注册功能开发中，敬请期待！");
}
