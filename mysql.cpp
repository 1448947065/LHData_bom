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

void MySql::connectDatabase(const QString &connName,
                            const QString &host,quint16 port,
                            const QString &user,const QString &pwd,
                            bool savePwd, bool saveConfig)
{
    if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        qCritical() << "QMYSQL驱动不可用！可用驱动：" << QSqlDatabase::drivers();
        return;
    }

    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase::removeDatabase(connName);
    }

    db = QSqlDatabase::addDatabase("QMYSQL", connName);
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(pwd);
    //db.setDatabaseName(connName);

    if (!db.open()) {
        qCritical() << "连接数据库失败：" << db.lastError().text();
        return;
    }

    qDebug() << "成功连接到已有数据库 lhlist";
}

QSqlTableModel* MySql::getModel(const QString &tableName)
{
    QSqlTableModel *model = new QSqlTableModel(nullptr, db);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}

void MySql::handleDbInfo(const QString &connName, const QString &host,
                 quint16 port, const QString &user, const QString &pwd,
                 bool savePwd, bool saveConfig)
{
        qDebug() << "==== 收到数据库连接信号 ====";
    connectDatabase(connName, host,
                    port, user, pwd, savePwd, saveConfig);
    qDebug()<<"1234";

}

int MySql::insertDatabase()
{
    return 0;
}






