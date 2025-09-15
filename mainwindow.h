#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//system.h
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
//user.h
#include <mysql.h>
#include "titlebar.h"
//class
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
#define global_menu 0
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //titlePresent();
private slots:

    void on_btn_sql_mar_clicked();
    void on_btn_bom_clicked();

protected:
    void InitUi();
    void ConnectSlot();
    void test();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
