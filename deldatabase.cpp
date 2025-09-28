#include "deldatabase.h"
#include "ui_deldatabase.h"

DelDataBase::DelDataBase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelDataBase)
{
    ui->setupUi(this);
}

DelDataBase::~DelDataBase()
{
    delete ui;
}
