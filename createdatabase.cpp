#include "createdatabase.h"
#include "ui_createdatabase.h"

CreateDatabase::CreateDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDatabase)
{
    ui->setupUi(this);
}

CreateDatabase::~CreateDatabase()
{
    delete ui;
}

void CreateDatabase::on_btn_cre_ok_clicked()
{
    QString dbName = ui->lineEdit_dbName->text().trimmed();
    QString charset = ui->combo_charset->currentText();
    QString collation = ui->combo_collation->currentText();

    if (dbName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Database name cannot be empty!"));
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("123456");
    //db.setDatabaseName("mysql");  // 先连系统库，不要写新库名

    if (!db.open()) {
        QMessageBox::critical(this, "Error", "Failed to connect: " + db.lastError().text());
        return;
    }
    QString sql = QString("CREATE DATABASE `%1` CHARACTER SET %2 COLLATE %3;")
                      .arg(dbName, charset, collation);

    QSqlQuery query;
    if (!query.exec(sql)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to create database:\n%1")
                             .arg(query.lastError().text()));
    } else {
        QMessageBox::information(this, tr("Success"),
                                 tr("Database %1 created successfully!").arg(dbName));
        accept();   // OK 点击成功时关闭对话框
    }
}

