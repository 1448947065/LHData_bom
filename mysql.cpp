#include "mysql.h"
#include "ui_form.h"
namespace Ui {
class MySql;
}
MySql::MySql(QWidget *parent)
            : QWidget(parent),
            ui(new Ui::MySql)
{
    ui->setupUi(this);
}
MySql::~MySql()
{
    delete ui;
}
