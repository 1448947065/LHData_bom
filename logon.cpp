#include "logon.h"
#include "ui_logon.h"

logon::logon(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logon)
{
    ui->setupUi(this);
}

logon::~logon()
{
    delete ui;
}

void logon::on_pushButton_clicked()
{
    emit reg_end();
}

void logon::registersult(bool result, QString resultt)
{
    if (result) {
        // 注册成功提示
        QMessageBox::information(
            this,                   // 父窗口指针
            "注册成功",             // 标题
            resultt,               // 提示内容
            QMessageBox::Ok,       // 按钮类型
            QMessageBox::Ok        // 默认选中按钮
        );

        // 注册成功后可以关闭窗口或清空输入框
        this->close();            // 关闭当前注册窗口
        // 或者：
        // ui->usernameEdit->clear();
        // ui->passwordEdit->clear();
    } else {
        // 注册失败提示
        QMessageBox::critical(
            this,
            "注册失败",
            resultt,
            QMessageBox::Ok,
            QMessageBox::Ok
        );

        // 失败时让用户名输入框重新获得焦点
//        ui->usernameEdit->setFocus();
    }
}
