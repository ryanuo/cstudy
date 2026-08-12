#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class Calculator : public QDialog
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);

private slots:
    void onCalculate();
    void onReset();

private:
    double calculate();

    QLineEdit *m_input1;
    QLineEdit *m_input2;
    QComboBox *m_opCombo;
    QPushButton *m_calcBtn;
    QPushButton *m_resetBtn;
    QLabel *m_resultLabelTitle;
    QLabel *m_resultLabel;
    QPushButton *m_quitBtn;
};

#endif