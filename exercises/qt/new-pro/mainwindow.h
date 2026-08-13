#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setCurrentUser(const QString &username);   // 状态栏显示当前登录用户

private slots:
    void on_saveButton_clicked();    // 保存
    void on_resetButton_clicked();   // 重置

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
