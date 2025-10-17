#include "logon.h"
#include "ui_logon.h"

logon::logon(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logon)
{
    ui->setupUi(this);
    connect(this, &logon::reg_str, m_sql, &MySql::logon_register);
}

logon::~logon()
{
    delete ui;
}

void logon::on_pushButton_clicked()
{
    QString reg_usr = ui->line_usr->text();
    QString reg_pwd = ui->line_pwd1->text();
    emit reg_str(reg_usr,reg_pwd);
    qDebug("123r5");
}

void logon::registersult(bool result, QString resultt)
{

//    } else {
//        // 注册失败提示
//        QMessageBox::critical(
//            this,
//            "注册失败",
//            resultt,
//            QMessageBox::Ok,
//            QMessageBox::Ok
//        );

//        // 失败时让用户名输入框重新获得焦点
////        ui->usernameEdit->setFocus();
//    }
}
void logon::suc_result(bool result)
{
    if (result) {
        QMessageBox::information(this, "提示", "注册成功！");
        ui->label_4->setText("suc");
    }
    else {
        QMessageBox::information(this, "提示", "注册失败！");
    }
}
