#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QSlider;
class QSpinBox;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onCountClicked();

private:
    QLabel      *m_valueLabel;
    QSlider     *m_slider;
    QSpinBox    *m_spinBox;
    QPushButton *m_countButton;
    int          m_count = 0;
};

#endif // MAINWINDOW_H