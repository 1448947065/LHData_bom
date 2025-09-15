#ifndef MYSQl_H
#define MYSQl_H

#include <QWidget>
#include <QXlsx/Worksheet>
#include <QXlsx/Workbook>

namespace Ui {
class MySql;
}

class MySql : public QWidget
{
    Q_OBJECT

public:
    explicit MySql(QWidget *parent = nullptr);
    ~MySql();

private:
    Ui::MySql *ui;
};

#endif

