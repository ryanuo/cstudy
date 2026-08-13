#include "registerdialog.h"
#include "ui_registerdialog.h"

#include "dbhelper.h"

#include <QMessageBox>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("用户注册"));
    setModal(true);

    // 与登录窗口一致的输入框样式
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

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

QString RegisterDialog::username() const
{
    return m_username;
}

// 注册：校验后写入 SQLite(密码存 SHA-256 哈希)
void RegisterDialog::on_registerButton_clicked()
{
    const QString username = ui->usernameEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();
    const QString confirm = ui->confirmEdit->text();

    // 非空校验
    if (username.isEmpty() || password.isEmpty() || confirm.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("注册失败"), QStringLiteral("用户名和密码不能为空！"));
        return;
    }
    // 两次密码一致性
    if (password != confirm)
    {
        QMessageBox::warning(this, QStringLiteral("注册失败"), QStringLiteral("两次输入的密码不一致！"));
        ui->confirmEdit->clear();
        ui->confirmEdit->setFocus();
        return;
    }
    // 用户名查重
    if (db::userExists(username))
    {
        QMessageBox::warning(this, QStringLiteral("注册失败"), QStringLiteral("该用户名已被注册！"));
        return;
    }
    // 入库(密码哈希后存储)
    if (!db::addUser(username, db::hashPassword(password)))
    {
        QMessageBox::critical(this, QStringLiteral("注册失败"), QStringLiteral("写入数据库失败！"));
        return;
    }

    m_username = username;
    QMessageBox::information(this, QStringLiteral("注册成功"), QStringLiteral("注册成功，请登录！"));
    accept();
}

// 取消
void RegisterDialog::on_cancelButton_clicked()
{
    reject();
}
