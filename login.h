#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <mainwindow.h>
#include <logon.h>
namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();
    void reg_callback();
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
};

#endif // LOGIN_H
