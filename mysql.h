#ifndef MYSQl_H
#define MYSQl_H

#include <QWidget>
#include <QDebug>
#include <QDateTime>
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
private:
    int row = 1;
    int col = 1;
    QSqlDatabase db;
protected:
    void recv_excel();

};

#endif

