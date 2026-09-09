#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 整体美化（与登录框同主题：Ant Design 风，柔和蓝）
    setStyleSheet(R"(
        QMainWindow {
            background: #f5f7fa;
        }
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: bold;
            color: #1f2329;
        }
        QLabel#cardBasicTitle, QLabel#cardExtraTitle {
            font-size: 14px;
            font-weight: bold;
            color: #1f2329;
        }
        QLabel {
            font-size: 13px;
            color: #4e5969;
        }
        QFrame#cardBasic, QFrame#cardExtra {
            background: #ffffff;
            border: 1px solid #e5e6eb;
            border-radius: 8px;
        }
        QLineEdit, QTextEdit, QDateEdit,QSpinBox {
            border: 1px solid #d9d9d9;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
            color: #333333;
            background: #ffffff;
            selection-background-color: #d6e4ff;
            selection-color: #333333;
        }
        QLineEdit:hover, QTextEdit:hover, QDateEdit:hover, QComboBox:hover,QSpinBox:hover  {
            border-color: #b3b3b3;
        }
        QLineEdit:focus, QTextEdit:focus, QDateEdit:focus, QComboBox:focus,QSpinBox:focus {
            border-color: #409eff;
        }

        QSpinBox::up-button:hover,
        QSpinBox::down-button:hover {
            background: #f5f7fa;
        }

        QComboBox {
            border: 1px solid #d9d9d9;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
            color: #333333;
            background: #ffffff;
            selection-background-color: #d6e4ff;
            selection-color: #333333;
        }
        QComboBox::drop-down {
            border: none;
            width: 26px;
        }
        QPushButton#saveButton {
            background: #409eff;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 8px 28px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#saveButton:hover {
            background: #66b1ff;
        }
        QPushButton#saveButton:pressed {
            background: #3a8ee6;
        }
        QPushButton#resetButton {
            background: #ffffff;
            color: #4e5969;
            border: 1px solid #d9d9d9;
            border-radius: 6px;
            padding: 8px 28px;
            font-size: 13px;
        }
        QPushButton#resetButton:hover {
            border-color: #409eff;
            color: #409eff;
        }
        QRadioButton {
            font-size: 13px;
            color: #4e5969;
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 14px;
            height: 14px;
        }
        QRadioButton::indicator:unchecked {
            border: 1px solid #d9d9d9;
            border-radius: 7px;
            background: #ffffff;
        }
        QRadioButton::indicator:checked {
            border: 4px solid #409eff;
            border-radius: 7px;
            background: #ffffff;
        }
        QStatusBar {
            background: #ffffff;
            border-top: 1px solid #e5e6eb;
            color: #4e5969;
            font-size: 12px;
        }
    )");

    // 起止日期默认值：今天 ~ 一周后
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate().addDays(7));

    // 默认选中「公开」
    ui->radioButton->setChecked(true);

    // 选中「部分公开」时显示同事下拉，否则隐藏
    connect(ui->radioButton_3, &QRadioButton::toggled,
            ui->colleagueCombo, &QWidget::setVisible);

    setCurrentUser(QString());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 状态栏显示当前登录用户
void MainWindow::setCurrentUser(const QString &username)
{
    if (username.isEmpty())
        statusBar()->showMessage("未登录");
    else
        statusBar()->showMessage(QString("当前用户：%1").arg(username));
}

// 保存：校验标题非空后弹提示（演示，未接存储）
void MainWindow::on_saveButton_clicked()
{
    if (ui->lineEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先填写目标标题");
        ui->lineEdit->setFocus();
        return;
    }
    QMessageBox::information(this, "保存", "目标保存成功（演示）");
}

// 重置：清空所有输入，恢复默认值
void MainWindow::on_resetButton_clicked()
{
    ui->lineEdit->clear();
    ui->spinBox->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate().addDays(7));
    ui->radioButton->setChecked(true);
}
