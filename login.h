#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <mainwindow.h>
#include <logon.h>
#include "mysql.h"

extern QString g_usr_n;
namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

signals:
    void check_pwd(QString userName, QString pwd);
private slots:
    void on_btn_sign_in_clicked();

    void on_btn_sign_up_clicked();

private:
    Ui::login *ui;
    QString UserName;
    QString PassWord;
    MainWindow mainWindow;
    logon *logondialog = new logon();
    MySql *l_sql = new MySql;
};

#endif // LOGIN_H
