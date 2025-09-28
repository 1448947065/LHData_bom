#ifndef DELDATABASE_H
#define DELDATABASE_H

#include <QDialog>

namespace Ui {
class DelDataBase;
}

class DelDataBase : public QDialog
{
    Q_OBJECT

public:
    explicit DelDataBase(QWidget *parent = nullptr);
    ~DelDataBase();

private:
    Ui::DelDataBase *ui;
};

#endif // DELDATABASE_H
