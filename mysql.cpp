#include "mysql.h"
namespace Ui {
class MySql;
}
MySql::MySql(QWidget *parent) : QWidget(parent)
{

}
void MySql::recv_excel()
{

}
MySql::~MySql()
{
}
bool MySql::initializeDatabase(const QVector<QVector<QString>> columnData)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("123456");
    if (!db.open()) {
        qDebug() << "MySQL服务器连接失败：" << db.lastError().text();
        return false;
    }
    qDebug() << "MySQL服务器连接成功！";

    QSqlQuery checkDb(db);
    if (!checkDb.exec("CREATE DATABASE IF NOT EXISTS lhlist "
                     "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")) {
        qDebug() << "创建数据库失败：" << checkDb.lastError().text();
        return false;
    }
    qDebug() << "数据库 lhlist 创建/验证成功";

    db.close();
    db.setDatabaseName("lhlist");
    if (!db.open()) {
        qDebug() << "连接数据库 lhlist 失败：" << db.lastError().text();
        return false;
    }
    qDebug() << "成功连接到数据库 lhlist";

    QSqlQuery createQuery(db);
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS lhbom (
            id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
            serial_no VARCHAR(20) NOT NULL,
            category VARCHAR(50) NOT NULL,
            component_name VARCHAR(100) NOT NULL,
            model_specification VARCHAR(100) NOT NULL,
            manufacturer VARCHAR(100) NOT NULL DEFAULT '长春半导体有限公司',
            created_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
            UNIQUE INDEX uq_serial_no (serial_no),
            INDEX idx_category (category),
            INDEX idx_model_spec (model_specification)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!createQuery.exec(createTableSql)) {
        qDebug() << "创建表失败：" << createQuery.lastError().text();
        return false;
    }
    qDebug() << "表 lhbom 创建成功！";

    if (columnData.size() < 5) {
        qDebug() << "错误：需要至少5列数据";
        return false;
    }

    if (columnData[0].isEmpty()) {
        qDebug() << "提示：无数据需要插入";
        return true;
    }

    int rowCount = columnData[0].size();
    QSqlQuery insertQuery(db);
    insertQuery.prepare(R"(
        INSERT INTO lhbom
        (serial_no, category, component_name, model_specification, manufacturer)
        VALUES (?, ?, ?, ?, ?)
    )");

    db.transaction();

        for (int row = 1; row < rowCount; ++row) {

            if (columnData[0][row].isEmpty()) {
                throw std::runtime_error(QString("第%1行: serial_no不能为空").arg(row+1).toStdString());
            }

            int insertedCount = 0;
            for (int row = 0; row < columnData[0].size(); ++row) {

                insertQuery.addBindValue(columnData[0][row]); // serial_no
                insertQuery.addBindValue(columnData[1][row]); // category
                insertQuery.addBindValue(columnData[2][row]); // component_name
                insertQuery.addBindValue(columnData[3][row]); // model_specification
                insertQuery.addBindValue(columnData[4][row]); // manufacturer

                if (!insertQuery.exec()) {
                    qDebug() << "插入行" << row+1 << "失败：" << insertQuery.lastError().text();

                    continue;
                }
                insertedCount++;

                if (row % 100 == 0) {
                    qDebug() << "已插入" << insertedCount << "/" << columnData[0].size() << "行";
                }
            }

            if (insertedCount > 0) {
                if (db.commit()) {
                    qDebug() << "成功提交事务，插入" << insertedCount << "行数据";
                    return true;
                } else {
                    qDebug() << "提交事务失败：" << db.lastError().text();
                    db.rollback();
                    return false;
                }
            } else {
                qDebug() << "没有数据被插入，回滚事务";
                db.rollback();
                return false;
            }
        }
}

int MySql::insertDatabase()
{
    return 0;
}






