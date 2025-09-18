#ifndef MYSQl_H
#define MYSQl_H

#include <QWidget>
#include <QDebug>
#include <QDateTime>
#include <QSqlTableModel>
#include <connection.h>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
namespace Ui {
class MySql;
}

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
    bool initializeDatabase(const QVector<QVector<QString>> columnData);
    int insertDatabase();
    QSqlTableModel* getModel(const QString &tableName);
public slots:
    void handleDbInfo(const QString &connName, const QString &host,
                     quint16 port, const QString &user, const QString &pwd,
                     bool savePwd, bool saveConfig);
private:
    int row = 1;
    int col = 1;
    QSqlDatabase db;
//    Connection* cmysql = new Connection(this);
protected:
    void recv_excel();
    void connectDatabase(const QString &connName, const QString &host,
                        quint16 port, const QString &user, const QString &pwd,
                        bool savePwd, bool saveConfig);

};

#endif

