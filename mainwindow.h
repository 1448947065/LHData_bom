#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//system.h
#include <QGraphicsDropShadowEffect>
#include <QStandardItemModel>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QTreeView>
#include <QIcon>
//user.h
#include <mysql.h>
#include "lhxlsx.h"
#include "titlebar.h"
#include <connection.h>
#include <createdatabase.h>
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
    MySql *dbManager = new MySql(this);
private slots:

    void on_btn_sql_mar_clicked();
    void on_btn_bom_clicked();
    void on_btn_connection_clicked();

    void on_btn_search_clicked();

    void on_btn_create_clicked();
    void onTreeViewContextMenu(const QPoint &pos);
signals:
    bool mysqlCreate(const QVector<QVector<QString>> columnData);

protected:
    void InitUi();
    void ConnectSlot();
    void test();
    void insert();
    void initiazeTreeView();
    bool eventFilter(QObject *obj, QEvent *event);
    void loadAllConnections(QStandardItem *parentItem);
    void createNewConnection();
    void saveConfigToIni(const QString &connName, const QString &host, quint16 port,
                                    const QString &user, const QString &pwd, bool savePwd, bool saveConfig);
    QStandardItem* findConnectionItem(const QString &connName);
    void connectToDatabase(const QString &connName);
    void onConnectionDoubleClicked(const QModelIndex &index);
    void loadSavedConnections(QStandardItem *parentItem);
    void setTableStyle();
private:
    Ui::MainWindow *ui;
    Connection *con = new Connection;
    CreateDatabase *cre = new CreateDatabase;
    QStandardItemModel *treeModel = new QStandardItemModel;
    QTreeView* treeView = new QTreeView;
};
#endif // MAINWINDOW_H
