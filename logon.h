#ifndef LOGON_H
#define LOGON_H

#include <QDialog>
#include <QObject>
#include <QMessageBox>
namespace Ui {
class logon;
}

class logon : public QDialog
{
    Q_OBJECT

public:
    explicit logon(QWidget *parent = nullptr);
    ~logon();
    void registersult(bool result, QString resultt);
signals:
    void reg_end();
private slots:
    void on_pushButton_clicked();

private:
    Ui::logon *ui;
};

#endif // LOGON_H
