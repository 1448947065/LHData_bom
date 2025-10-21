#include "mysql.h"
#include "exportinsert.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QSet>
namespace Ui {
class MySql;
}
MySql::MySql(QWidget *parent) : QWidget(parent)
{
//        m_logon = new logon(this);

}
void MySql::recv_excel()
{

}

MySql::~MySql()
{

}

// 保留原表头（中文/空格都允许），只做去重；为空时给个友好占位
static QString keepHeader(const QString &raw, QSet<QString> &used) {
    QString s = raw.trimmed();
    if (s.isEmpty()) s = QString("列%1").arg(used.size() + 1);  // 空表头 -> 列1/列2...
    QString base = s;
    int k = 2;
    while (used.contains(s)) {
        s = base + "_" + QString::number(k++);                  // 重名 -> 名称_2/_3...
    }
    used.insert(s);
    return s;
}

// 反引号安全包裹（可用于中文/含空格列名）
static QString backtick(const QString &ident) {
    QString s = ident;
    s.replace("`", "``");
    return QString("`%1`").arg(s);
}

bool MySql::initializeDatabase(const QString &cnName, const QString &DataBaseName, const QString &host,
                               const QString &user, const QString &userpwd, const QVector<QVector<QString>> &columnData)
{
    qDebug() << "连接状态详情："
             << "是否有效：" << db.isValid()
             << "是否打开：" << db.isOpen()
             << "数据库名：" << db.databaseName()
             << "最后错误：" << db.lastError().text();

    // --- 连接 ---
    if (!db.isOpen()) {
        QSqlDatabase::removeDatabase(db.connectionName());
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(host);
        db.setPort(3306);
        db.setUserName(user);
        db.setPassword(userpwd);
        db.setDatabaseName(DataBaseName);

        if (!db.open()) {
            qDebug() << "强制重连失败：" << db.lastError().text();
            QMessageBox::critical(nullptr, "数据库连接失败",
                                  "无法连接到数据库（重连失败）。\n错误信息：\n" + db.lastError().text());
            return false;
        }
    }
//    if (db.isOpen()) {
//        qDebug() << "数据库已连接，无需重复连接";
//        return true;  // 如果已经连接，跳过执行
//    }
    db.setDatabaseName(DataBaseName);
    if (!db.isOpen() && !db.open()) {
        qDebug() << "打开数据库失败:" << db.lastError().text();
        QMessageBox::critical(nullptr, "数据库打开失败",
                              "无法打开指定数据库。\n错误信息：\n" + db.lastError().text());
        return false;
    }
    qDebug() << "已连接到数据库:" << db.databaseName();

    // --- 基本校验：要求至少1行表头+1行数据，且按“列”为外层 ---
    if (columnData.isEmpty()) {
        qDebug() << "错误：没有任何列";
        return false;
    }

    const int colCount = columnData.size();
    const int rowCount = columnData[0].size();
    if (rowCount <= 1) {
        qDebug() << "提示：无数据需要插入";
        return true; // 只有表头
    }

    // === 1) 读取 Excel 表头并规范化为 SQL 标识符 ===
    QStringList headersRaw;
    headersRaw.reserve(colCount);
    QStringList headersSql;
    headersSql.reserve(colCount);
    QSet<QString> used;

    for (int c = 0; c < colCount; ++c) {
        QString head = columnData[c].isEmpty() ? QString() : columnData[c][0].trimmed();
        headersRaw << head;
        headersSql << keepHeader(head, used);   // 中文原样保留
    }
    qDebug() << "Excel 表头(raw):" << headersRaw;
    qDebug() << "SQL 列名(safe):" << headersSql;

    auto tableExists = [&](const QString &tbl)->bool {
        QSqlQuery q(db);
        q.prepare("SHOW TABLES LIKE ?");
        q.addBindValue(tbl);
        if (!q.exec()) return false;
        return q.next();
    };

    const QString tableName = "NonCompliant";
    const QString tableNameQuoted = backtick(tableName);

    // === 3) 若表不存在：按表头一次性建表 ===
    if (!tableExists(tableName)) {
        QStringList colDefs;
        colDefs << "id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY";
        for (const QString &col : headersSql) {
            colDefs << QString("%1 VARCHAR(255) NULL").arg(backtick(col));
        }
        colDefs << "created_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP";
        colDefs << "updated_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP";

        const QString createSql = QString(
            "CREATE TABLE %1 (\n  %2\n) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"
        ).arg(tableNameQuoted, colDefs.join(",\n  "));

        QSqlQuery q(db);
        if (!q.exec(createSql)) {
            qDebug() << "创建表失败：" << q.lastError().text();
            QMessageBox::critical(nullptr, "创建表失败",
                                  "无法创建表 compliant \n错误信息：\n" + q.lastError().text());
            return false;
        }
        qDebug() << "[DDL] 创建表 compliant 完成";
    } else {
        // === 4) 若表已存在：补齐缺失列（不删除旧列） ===
        QSet<QString> existing;
        {
            QSqlQuery q(db);
            if (!q.exec(QString("SHOW COLUMNS FROM %1").arg(tableNameQuoted))) {
                qDebug() << "读取现有列失败：" << q.lastError().text();
                QMessageBox::critical(nullptr, "读取列失败",
                                      "无法读取现有表结构。\n错误信息：\n" + q.lastError().text());
                return false;
            }
            while (q.next()) {
                existing.insert(q.value(0).toString().toLower());
            }
        }

        // 逐列补齐
        for (const QString &col : headersSql) {
            if (!existing.contains(col)) {
                const QString alterSql = QString("ALTER TABLE %1 ADD COLUMN %2 VARCHAR(255) NULL")
                                         .arg(tableNameQuoted, backtick(col));
                QSqlQuery aq(db);
                if (!aq.exec(alterSql)) {
                    qDebug() << "ALTER 失败：" << aq.lastError().text() << " SQL=" << alterSql;
                    QMessageBox::warning(nullptr, "补齐列失败",
                                         QString("新增列 %1 失败：\n%2").arg(col, aq.lastError().text()));
                    // 不中断，尽量继续
                } else {
                    qDebug() << "[DDL] 新增列" << col;
                }
            }
        }
    }

    // === 5) 构造动态 INSERT 语句 (INSERT INTO lhbom(`c1`,`c2`,...) VALUES(?,?,...)) ===
    QStringList colsQuoted;
    colsQuoted.reserve(headersSql.size());
    for (const QString &col : headersSql) colsQuoted << backtick(col);

    const QString insertSql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                              .arg(tableNameQuoted,
                                   colsQuoted.join(", "),
                                   QString("?, ").repeated(colsQuoted.size()).chopped(2));
    QSqlQuery insertQuery(db);
    if (!insertQuery.prepare(insertSql)) {
        qDebug() << "预处理失败：" << insertQuery.lastError().text() << " SQL=" << insertSql;
        QMessageBox::critical(nullptr, "预处理失败",
                              "无法预处理插入语句。\n错误信息：\n" + insertQuery.lastError().text());
        return false;
    }

    // === 6) 事务写入 ===
    if (!db.transaction()) {
        qDebug() << "开启事务失败:" << db.lastError().text();
        QMessageBox::critical(nullptr, "事务开启失败",
                              "无法开启数据库事务。\n错误信息：\n" + db.lastError().text());
        return false;
    }

    int insertedCount = 0;
    int skippedCount = 0;

    // 开始前
    emit progressChanged(0, rowCount, false, true, QString());

    auto safeGet = [&](int c, int r)->QVariant {
        if (c < 0 || c >= colCount) return QVariant();
        if (r < 0 || r >= columnData[c].size()) return QVariant();
        QString v = columnData[c][r].trimmed();
        if (v.isEmpty()) return QVariant(QVariant::String);
        return QVariant(v);
    };

    int batchSize = std::max(1, rowCount / 100);

    for (int r = 1; r < rowCount; ++r) {
        insertQuery.clear();
        insertQuery.prepare(insertSql);
        for (int c = 0; c < colCount; ++c) {
            insertQuery.addBindValue(safeGet(c, r));
        }
        if (!insertQuery.exec()) {
            qWarning() << "第" << (r+1) << "行插入失败：" << insertQuery.lastError().text();
            ++skippedCount;
            continue;
        }
        ++insertedCount;

        if (r % batchSize == 0 || r == rowCount - 1) {
            emit progressChanged(r, rowCount, false, true, QString());
        }

        if ((r & 0x3F) == 0) QCoreApplication::processEvents();
    }

    if (!db.commit()) {
        qDebug() << "提交事务失败：" << db.lastError().text();
        db.rollback();
        QMessageBox::critical(nullptr, "提交失败",
                              "事务提交失败，已回滚。\n错误信息：\n" + db.lastError().text());
        return false;
    }

//    qDebug() << QString("导入完成：成功 %1 行，跳过 %2 行。").arg(insertedCount).arg(skippedCount);
//    QMessageBox::information(nullptr, "导入结果",
//                             QString("导入完成：成功 %1 行，跳过 %2 行。").arg(insertedCount).arg(skippedCount));

    // 发送完成进度
    emit progressChanged(rowCount, rowCount, true, true,
                         tr("导入完成：成功 %1 行，跳过 %2 行。")
                         .arg(insertedCount).arg(skippedCount));
    return true;
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
    db.setDatabaseName(connName);

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

bool MySql::login_check_pwd(QString UserName, QString PassWord)
{
    qDebug() << "=== 登录校验开始 ===";

    UserName = UserName.trimmed();
    if (UserName.isEmpty() || PassWord.isEmpty()) {
        return false;
    }

    const QString connName = QString("conn_login_%1").arg(QDateTime::currentMSecsSinceEpoch());
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
        db.setHostName("192.168.1.56");
        db.setPort(3306);
        db.setUserName("remote_admin");
        db.setPassword("123456");
        db.setDatabaseName("lhlist");

        if (!db.open()) {
            qCritical() << "登录时数据库连接失败:" << db.lastError().text();
            return false;
            goto CLEANUP;
        }

        // 2) 取出 password 字段：形如 "salt|hexhash"
        QString dbPassword;
        {
            QSqlQuery q(db);
            q.prepare("SELECT password FROM users WHERE username = ? LIMIT 1");
            q.addBindValue(UserName);
            if (!q.exec()) {
                qCritical() << "查询用户失败:" << q.lastError().text();
                return false;
                goto CLEANUP;
            }
            if (!q.next()) {
                return false;
                goto CLEANUP;
            }
            dbPassword = q.value(0).toString();
        }

        // 3) 拆分出 salt 和 storedHash
        const int sep = dbPassword.indexOf('|');
        if (sep <= 0) {
            qWarning() << "存储口令格式异常:" << dbPassword;
            return false;
            goto CLEANUP;
        }
        const QString salt       = dbPassword.left(sep);
        const QString storedHash = dbPassword.mid(sep + 1);

        // 4) 重新计算哈希：SHA256( PassWord + salt )
        const QString calcHash = QCryptographicHash::hash(
            (PassWord + salt).toUtf8(),
            QCryptographicHash::Sha256
        ).toHex();

        // 5) 常量时间比较，防止时间侧信道
        auto ctEqual = [](const QByteArray& a, const QByteArray& b) {
            if (a.size() != b.size()) return false;
            unsigned char diff = 0;
            for (int i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
            return diff == 0;
        };
        const bool ok = ctEqual(storedHash.toLatin1(), calcHash.toLatin1());

        if (!ok) {
            return false;
            goto CLEANUP;
        }

        // 6) 更新 last_login
        {
            QSqlQuery u(db);
            u.prepare("UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE username = ?");
            u.addBindValue(UserName);
            if (!u.exec()) {
                qWarning() << "更新 last_login 失败:" << u.lastError().text();
                // 不阻断登录成功，只记录告警
            }
        }

        // 7) 成功
        qInfo() << "用户登录成功:" << UserName;
        g_usr_n = UserName;
        return true;
    }

CLEANUP:
    QSqlDatabase::removeDatabase(connName);
    qDebug() << "=== 登录校验结束 ===";
}


void MySql::logon_register(QString UserName, QString PassWord)
{
    qDebug() << "=== 注册函数开始 ===";

    // 1. 输入验证
    UserName = UserName.trimmed();
    if (UserName.isEmpty() || PassWord.isEmpty()) {
        emit registerResult(false, "用户名和密码不能为空");
        qDebug() << "输入为空";
        return;
    }
    if (UserName.length() < 4 || UserName.length() > 50) {
        emit registerResult(false, "用户名长度需为4-50个字符");
        qDebug() << "用户名长度不符";
        return;
    }
    if (PassWord.length() < 8) {
        emit registerResult(false, "密码长度至少8位");
        qDebug() << "密码长度不足";
        return;
    }

    // 2. 创建数据库连接
    QString connName = QString("conn_%1").arg(QDateTime::currentMSecsSinceEpoch());
    {
        qDebug() << "准备连接数据库:" << connName;
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
        db.setHostName("192.168.1.56");
        db.setPort(3306);
        db.setUserName("remote_admin");
        db.setPassword("123456");
        db.setDatabaseName("lhlist");

        qDebug() << "驱动可用:" << QSqlDatabase::drivers();

        if (!db.open()) {
            qCritical() << "数据库连接失败:" << db.lastError().text();
            emit registerResult(false, "数据库连接失败");
            goto CLEANUP;
        }
        qDebug() << "数据库连接成功";

        // 3. 检查用户名是否存在
        {
            QSqlQuery checkQuery(db);
            checkQuery.prepare("SELECT 1 FROM users WHERE username = ?");
            checkQuery.addBindValue(UserName);
            qDebug() << "执行用户名检查:" << UserName;
            if (!checkQuery.exec()) {
                qCritical() << "用户名检查失败:" << checkQuery.lastError().text();
                emit registerResult(false, "系统繁忙，请稍后重试");
                goto CLEANUP;
            }
            if (checkQuery.next()) {
                qDebug() << "用户名已存在:" << UserName;
                emit registerResult(false, "该用户名已被注册");
                goto CLEANUP;
            }
        }

        // 4. 生成盐 + 哈希
        QString salt = QUuid::createUuid().toString().mid(1, 8);
        QString saltedPassword = PassWord + salt;
        QString passwordHash = QCryptographicHash::hash(
            saltedPassword.toUtf8(), QCryptographicHash::Sha256
        ).toHex();
        QString dbPassword = salt + "|" + passwordHash;

        qDebug() << "生成密码哈希:" << dbPassword;

        // 5. 插入用户
        {
            QSqlQuery insertQuery(db);
            insertQuery.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
            insertQuery.addBindValue(UserName);
            insertQuery.addBindValue(dbPassword);

            qDebug() << "执行插入语句:" << insertQuery.lastQuery();
            qDebug() << "绑定值:" << UserName << dbPassword;

            if (!insertQuery.exec()) {
                qCritical() << "注册失败:" << insertQuery.lastError().text()
                            << "错误码:" << insertQuery.lastError().nativeErrorCode();
                emit registerResult(false, "注册失败:" + insertQuery.lastError().text());
                goto CLEANUP;
            }
        }

        qInfo() << "用户注册成功:" << UserName;
        emit registerResult(true, "注册成功");
    }

CLEANUP:
    // 🔥 注意：removeDatabase 一定要在所有 Query 对象销毁后执行
    qDebug() << "清理连接:" << connName;
    QSqlDatabase::removeDatabase(connName);
    qDebug() << "=== 注册函数结束 ===";
}
