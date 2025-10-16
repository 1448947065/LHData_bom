#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//system.h
#include <QGraphicsDropShadowEffect>
#include <QStandardItemModel>
#include <QPropertyAnimation>
#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QDebug>

#include <QSqlError>
#include <QSqlQuery>
#include <QTreeView>
#include <QIcon>
#include <QLabel>
//user.h
#include <mysql.h>
#include "lhxlsx.h"
#include "titlebar.h"
#include <connection.h>
#include <createdatabase.h>
#include <deldatabase.h>
#include <exportinsert.h>
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
    void onConnectionSaved(const QString &connName);
    void on_btn_sql_mar_clicked();
    void on_btn_bom_clicked();
    void on_btn_connection_clicked();

    void on_btn_search_clicked();

    void on_btn_create_clicked();
    void onTreeViewContextMenu(const QPoint &pos);
    void on_btn_del_clicked();

    void on_btn_export_clicked();

    void on_btn_serach_tab_clicked();

    void on_btn_hyper_clicked();

signals:
    void dbmes(QString connName, QString dbName);

protected:
    void InitUi();
    void ConnectSlot();
    void insertDatabase();
    void insert();
    void initiazeTreeView();
    void loadAllConnections(QStandardItem *parentItem);
    void createNewConnection();
    void saveConfigToIni(const QString &connName, const QString &host, quint16 port,
                                    const QString &user, const QString &pwd, bool savePwd, bool saveConfig);
    QStandardItem* findConnectionItem(const QString &connName);
    void connectToDatabase(const QString &connName);
    void onConnectionDoubleClicked(const QModelIndex &index);
    void loadSavedConnections(QStandardItem *parentItem);
    void setTableStyle();
    void setLabelStatus();
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    bool deleteConnection(const QString &connName);
    void selectData();
private:
    Ui::MainWindow *ui;
    Connection *con = new Connection;
    CreateDatabase *cre = new CreateDatabase;
    DelDataBase *delui = new DelDataBase;
    ExportInsert *Insert = new ExportInsert;
    QStandardItemModel *treeModel = new QStandardItemModel;
    QTreeView* treeView = new QTreeView;
    QLabel* dataBaseStatus;
    QLabel* timeLabel;
    QWidget *card;
    QGraphicsDropShadowEffect *effect;
    QSortFilterProxyModel* m_proxy = nullptr;
    int m_findRow = -1;
    int m_findCol = -1;
    QString m_findTerm;
    QSqlDatabase db;
};
#endif // MAINWINDOW_H
