#ifndef MYSQl_H
#define MYSQl_H

#include <QWidget>
#include <QDebug>
#include <QDateTime>
#include <QSqlTableModel>
#include <connection.h>
//#include <exportinsert.h>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
//#include "logon.h"
namespace Ui {
class MySql;

}
class ExportInsert;
class MySql : public QWidget
{
    Q_OBJECT

public:
    explicit MySql(QWidget *parent = nullptr);
    ~MySql();
    int id;
    QString serialNumber;
    QString category;
    QString componentName;
    QString modelSpec;
    QString manufacturer;
    QDateTime createdAt;
    QDateTime updatedAt;
    //bool initializeDatabase(const QString &cnName, const QString &DataBaseName, const QString &host, const QString &user,
     //                              const QString userpwd, const QVector<QVector<QString>> &columnData);
    bool initializeDatabase(const QString &cnName, const QString &DataBaseName, const QString &host,
                                   const QString &user, const QString &userpwd, const QVector<QVector<QString>> &columnData);
    int insertDatabase();
    QSqlTableModel* getModel(const QString &tableName);
    bool login_check_pwd(QString UserName, QString PassWord);
    void logon_register(QString UserName, QString PassWord);
public slots:
    void handleDbInfo(const QString &connName, const QString &host,
                     quint16 port, const QString &user, const QString &pwd,
                     bool savePwd, bool saveConfig);
signals:
    void progressChanged(int current, int total, bool finished, bool ok, const QString &msg);
    void registerResult(bool result, QString resultSig);
private:
    int row = 1;
    int col = 1;
    QSqlDatabase db;
//    logon* m_logon = new logon;
protected:
    void recv_excel();
    void connectDatabase(const QString &connName, const QString &host,
                        quint16 port, const QString &user, const QString &pwd,
                        bool savePwd, bool saveConfig);

};

#endif

