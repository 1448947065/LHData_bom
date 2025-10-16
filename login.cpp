#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    connect(logondialog, &logon::reg_end, this, &login::reg_callback);
}

login::~login()
{
    delete ui;
}

void login::on_btn_sign_in_clicked()
{
    UserName = ui->line_username->text().trimmed();
    PassWord = ui->line_password->text();

    if(UserName.isEmpty() || PassWord.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空");
        return;
    }
    emit check_pwd(UserName, PassWord);
    mainWindow.show();
    this->hide();
}

void login::on_btn_sign_up_clicked()
{
    logondialog->show();
    this->hide();
}

void login::reg_callback()
{
    this->show();
    logondialog->hide();
}
