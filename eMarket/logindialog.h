#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QSqlQuery>                // 查询MySQL的库
#include <QMessageBox>
#include <QCryptographicHash>       // 包含MD5算法库

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    // QString strToMD5(QString str);          // 将口令字符串转换为MD5加密

private slots:
    void on_loginPushButton_clicked();      // 登录按钮事件
    void on_exitPushButton_clicked();       // 退出按钮事件

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
