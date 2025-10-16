#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
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
    bool result = l_sql->login_check_pwd(UserName, PassWord);
    if (result) {
        g_usr_n = UserName;
        mainWindow.show();
        this->hide();
    }
    else {
        QMessageBox::warning(this, tr("警告"), tr("用户名或密码错误"));
    }
}

void login::on_btn_sign_up_clicked()
{
    logondialog->show();
    this->hide();
}
