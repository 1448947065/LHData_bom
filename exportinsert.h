#ifndef EXPORTINSERT_H
#define EXPORTINSERT_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include <QtXlsx>
#include <mysql.h>
struct DbConfig {
    QString host;
    int port;
    QString user;
    QString password;
    bool savePwd;
    bool saveConfig;
};
namespace Ui {
class ExportInsert;

}

class ExportInsert : public QDialog
{
    Q_OBJECT

public:
    explicit ExportInsert(QWidget *parent = nullptr);
    ~ExportInsert();
    MySql *dbManagerExport = new MySql(this);
    void updateProcess();
signals:
    bool mysqlCreate(const QString &cnName,
                     const QString &dbName,
                     const QString &host,
                     const QString &user,
                     const QString &password,
                     const QVector<QVector<QString>> &columnData);
private slots:
    void on_pushButton_clicked();

    void on_buttonBox_accepted();

private:
    Ui::ExportInsert *ui;
    QString filePath;
protected:
    DbConfig readDbConfig(const QString &connName);
};

#endif // EXPORTINSERT_H
