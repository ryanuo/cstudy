#include "LoginDialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
// 模拟校验用的默认账号
const QString kDefaultUser = QStringLiteral("admin");
const QString kDefaultPass = QStringLiteral("123456");
} // namespace

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("用户登录"));
    resize(360, 250);
    // ---- 表单行样式 ----
    const QString labelStyle =
        "QLabel { font-size: 14px; color: #555; }";
    const QString editStyle =
        "QLineEdit { font-size: 15px; border: 1px solid #d0d0d0;"
        "            border-radius: 6px; padding: 4px 10px; }"
        "QLineEdit:focus { border-color: #4a90d9; }";

    // ---- 用户名 ----
    auto *userLabel = new QLabel(QStringLiteral("用户名:"), this);
    userLabel->setStyleSheet(labelStyle);
    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText(QStringLiteral("请输入用户名，默认admin"));
    m_userEdit->setMinimumHeight(36);
    m_userEdit->setStyleSheet(editStyle);
    m_userEdit->setClearButtonEnabled(true); // 网页风格: 有内容时显示内嵌 ×

    // ---- 密码(默认密文显示) ----
    auto *passLabel = new QLabel(QStringLiteral("密  码:"), this);
    passLabel->setStyleSheet(labelStyle);
    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText(QStringLiteral("请输入密码，默认123456"));
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setMinimumHeight(36);
    m_passEdit->setStyleSheet(editStyle);
    m_passEdit->setClearButtonEnabled(true); // 网页风格: 有内容时显示内嵌 ×

    // ---- 显示密码开关 ----
    m_showPassCheck = new QCheckBox(QStringLiteral("显示密码"), this);
    m_showPassCheck->setStyleSheet(
        "QCheckBox { font-size: 13px; color: #666; }");

    // ---- 提示标签 ----
    m_hintLabel = new QLabel("", this);
    m_hintLabel->setMinimumHeight(20);
    m_hintLabel->setStyleSheet(
        "QLabel { font-size: 13px; color: #999; }");

    // ---- 登录(主题色, 突出) ----
    m_loginBtn = new QPushButton(QStringLiteral("登 录"), this);
    m_loginBtn->setMinimumHeight(28);
    m_loginBtn->setMinimumWidth(64);
    m_loginBtn->setStyleSheet(
        "QPushButton { font-size: 13px; color: white; background: #4a90d9;"
        "              border: none; border-radius: 5px; font-weight: bold; }"
        "QPushButton:hover { background: #3a7dc4; }"
        "QPushButton:pressed { background: #2f6aa8; }");

    // ---- 清空 / 退出(小号次级按钮) ----
    const QString secondBtnStyle =
        "QPushButton { font-size: 13px; background: #f5f5f5;"
        "              border: 1px solid #d0d0d0; border-radius: 5px; }"
        "QPushButton:hover { background: #e8e8e8; }";
    m_quitBtn = new QPushButton(QStringLiteral("退出"), this);
    m_quitBtn->setMinimumHeight(28);
    m_quitBtn->setMinimumWidth(64);
    m_quitBtn->setStyleSheet(secondBtnStyle);

    // 用户名 / 密码表单行: 标签 + 输入框
    auto *userRow = new QHBoxLayout;
    userRow->addWidget(userLabel);
    userRow->addWidget(m_userEdit, 1);

    auto *passRow = new QHBoxLayout;
    passRow->addWidget(passLabel);
    passRow->addWidget(m_passEdit, 1);

    // 按钮行: 右下角
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(m_loginBtn);
    btnRow->addWidget(m_quitBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);
    mainLayout->addLayout(userRow);
    mainLayout->addLayout(passRow);
    mainLayout->addWidget(m_showPassCheck);
    mainLayout->addWidget(m_hintLabel);
    mainLayout->addLayout(btnRow);

    // ---- 信号连接 ----
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_quitBtn, &QPushButton::clicked, this, &QDialog::close);
    connect(m_showPassCheck, &QCheckBox::toggled,
            this, &LoginDialog::onShowPassToggled);
}

// ---- 槽: 点击登录 ----
void LoginDialog::onLogin()
{
    const QString user = m_userEdit->text().trimmed();
    const QString pass = m_passEdit->text();

    if (user.isEmpty() || pass.isEmpty())
    {
        setHint(QStringLiteral("用户名和密码不能为空"), false);
        return;
    }
    if (user == kDefaultUser && pass == kDefaultPass)
    {
        setHint(QStringLiteral("登录成功"), true);
    }
    else
    {
        setHint(QStringLiteral("用户名或密码错误"), false);
    }
}

// ---- 槽: 显示密码开关 ----
void LoginDialog::onShowPassToggled(bool checked)
{
    // 勾选 → 明文, 取消 → 密文
    m_passEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

// ---- 提示标签: 成功绿色 / 失败红色 ----
void LoginDialog::setHint(const QString &text, bool ok)
{
    m_hintLabel->setText(text);
    m_hintLabel->setStyleSheet(
        QStringLiteral("QLabel { font-size: 13px; color: %1; }")
            .arg(ok ? QStringLiteral("#2e9e5b") : QStringLiteral("#d9534f")));
}
