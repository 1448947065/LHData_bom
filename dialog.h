#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtXlsx>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QtXlsx>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include "xlsxdocument.h"
#include <QDebug>
#include <QtCore>
#include <QtGlobal>
#include "mysql.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
signals:
    void sig_select();



private slots:
    void on_btn_openbom_clicked();
    //void on_btn_refresh_clicked();
//    void adjustWindowSize();
    void on_btn_comp_clicked();
    void onLayoutAdjusted();
    void on_btn_openmat_clicked();
    void resizeEvent(QResizeEvent *event);

    void on_btn_refresh_clicked();

    void on_btn_str_clicked();

private:
    Ui::Dialog *ui;
    QString bomFileFullName, bomFileFullName1, bomFileFullName2;
    QString matFileFullName;
    QXlsx::Document *bom, *bom1, *bom2;
    QXlsx::Document *mat;
    QXlsx::Document *result;
    bool checkManuFlag;
    bool checkDosageFlag;
    bool fillinManuFlag;
    MySql *m_sql = new MySql(this);
    QString m_connName;
};

#endif // DIALOG_H
