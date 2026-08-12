#include "Calculator.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

Calculator::Calculator(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("加减乘除"));
    resize(440, 260);

    // ---- 输入区: 带标签的表单行 ----
    m_input1 = new QLineEdit(this);
    m_input2 = new QLineEdit(this);
    for (auto *edit : {m_input1, m_input2})
    {
        edit->setAlignment(Qt::AlignRight);
        edit->setMinimumHeight(36);
        edit->setStyleSheet(
            "QLineEdit { font-size: 16px; border: 1px solid #d0d0d0;"
            "            border-radius: 6px; padding: 4px 10px; }"
            "QLineEdit:focus { border-color: #4a90d9; }");
        edit->setPlaceholderText(QStringLiteral("请输入数字"));
    }

    // ---- 运算符下拉 ----
    m_opCombo = new QComboBox(this);
    m_opCombo->addItem(QStringLiteral("+"), "+");
    m_opCombo->addItem(QStringLiteral("-"), "-");
    m_opCombo->addItem(QStringLiteral("*"), "*");
    m_opCombo->addItem(QStringLiteral("/"), "/");
    m_opCombo->setMinimumHeight(36);
    m_opCombo->setStyleSheet(
        "QComboBox { font-size: 15px; border: 1px solid #d0d0d0;"
        "           border-radius: 6px; padding: 4px 10px;"
        "           text-align: center;}"
        "QComboBox:hover { border-color: #4a90d9; }"
        "QComboBox::drop-down { border: none; width: 24px; }");

    // ---- 计算按钮(主题色, 突出, 小号) ----
    m_calcBtn = new QPushButton(QStringLiteral("计 算"), this);
    m_calcBtn->setMinimumHeight(28);
    m_calcBtn->setMinimumWidth(64);
    m_calcBtn->setStyleSheet(
        "QPushButton { font-size: 13px; color: white; background: #4a90d9;"
        "              border: none; border-radius: 5px; font-weight: bold; }"
        "QPushButton:hover { background: #3a7dc4; }"
        "QPushButton:pressed { background: #2f6aa8; }");

    // ---- 结果 ----
    m_resultLabelTitle = new QLabel(QStringLiteral("结果:"), this);
    m_resultLabelTitle->setStyleSheet(
        "QLabel { font-size: 14px; color: #555; }");
    m_resultLabel = new QLabel(QStringLiteral("-"), this);
    m_resultLabel->setMaximumHeight(32);

    // ---- 重置(小号) ----
    m_resetBtn = new QPushButton(QStringLiteral("重置"), this);
    m_resetBtn->setMinimumHeight(28);
    m_resetBtn->setMinimumWidth(64);
    m_resetBtn->setStyleSheet(
        "QPushButton { font-size: 13px; background: #f5f5f5;"
        "              border: 1px solid #d0d0d0; border-radius: 5px; }"
        "QPushButton:hover { background: #e8e8e8; }");

    // ---- 退出(小号) ----
    m_quitBtn = new QPushButton(QStringLiteral("退出"), this);
    m_quitBtn->setMinimumHeight(28);
    m_quitBtn->setMinimumWidth(64);
    m_quitBtn->setStyleSheet(
        "QPushButton { font-size: 13px; background: #f5f5f5;"
        "              border: 1px solid #d0d0d0; border-radius: 5px; }"
        "QPushButton:hover { background: #e8e8e8; }");

    // 输入行: 标签 + 输入框
    auto *row1 = new QHBoxLayout;
    row1->addWidget(m_input1, 1);
    row1->addWidget(m_opCombo);
    row1->addWidget(m_input2, 1);

    // 结果: 标题与数值框紧贴, 数值框按内容自适应宽度不撑开
    auto *resultRow = new QHBoxLayout;
    resultRow->setSpacing(6);
    resultRow->addWidget(m_resultLabelTitle);
    resultRow->addWidget(m_resultLabel);
    resultRow->addStretch(); // 剩余空间推到右侧

    // 按钮行: 右下角
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(m_calcBtn);
    btnRow->addWidget(m_resetBtn);
    btnRow->addWidget(m_quitBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(10);
    mainLayout->addLayout(row1);

    mainLayout->addLayout(resultRow);
    mainLayout->addLayout(btnRow);

    // ---- 信号连接 ----
    connect(m_calcBtn, &QPushButton::clicked, this, &Calculator::onCalculate);
    connect(m_resetBtn, &QPushButton::clicked, this, &Calculator::onReset);
    connect(m_quitBtn, &QPushButton::clicked, this, &QDialog::close);
}

// ---- 槽: 点击计算按钮 ----

void Calculator::onCalculate()
{
    const double result = calculate();
    // 除零时 calculate 已设置错误信息
    if (m_opCombo->currentData().toString() == "/" && m_input2->text().toDouble() == 0.0)
    {
        return;
    }
    m_resultLabel->setText(QStringLiteral("%1").arg(result));
}

// ---- 槽: 点击重置按钮 ----
void Calculator::onReset()
{
    m_input1->clear();
    m_input2->clear();
    m_resultLabel->setText(QStringLiteral("-"));
    m_opCombo->setCurrentIndex(0); // 回到 "加 (+)"
}

// ---- 运算逻辑 ----
double Calculator::calculate()
{
    const double a = m_input1->text().toDouble();
    const double b = m_input2->text().toDouble();
    const QString op = m_opCombo->currentData().toString();

    m_resultLabel->setStyleSheet("color: black;");
    if (op == "+")
    {
        return a + b;
    }
    if (op == "-")
    {
        return a - b;
    }
    if (op == "*")
    {
        return a * b;
    }
    if (op == "/")
    {
        if (b == 0.0)
        {
            m_resultLabel->setStyleSheet("color: red;");
            m_resultLabel->setText(QStringLiteral("错误(除数不能为0)"));
            return 0.0;
        }
        return a / b;
    }
    return 0.0;
}