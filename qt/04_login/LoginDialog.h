#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

private slots:
    void onLogin();
    void onShowPassToggled(bool checked);

private:
    void setHint(const QString &text, bool ok);

    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QCheckBox *m_showPassCheck;
    QPushButton *m_loginBtn;
    QPushButton *m_quitBtn;
    QLabel *m_avatarLabel;
    QLabel *m_hintLabel;
};

#endif
