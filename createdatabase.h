#ifndef CREATEDATABASE_H
#define CREATEDATABASE_H

#include <QDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
namespace Ui {
class CreateDatabase;
}

class CreateDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDatabase(QWidget *parent = nullptr);
    ~CreateDatabase();

private slots:

    void on_btn_cre_ok_clicked();

private:
    Ui::CreateDatabase *ui;
};

#endif // CREATEDATABASE_H
