#ifndef CONNECTION_H
#define CONNECTION_H

#include <QWidget>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
namespace Ui {
class Connection;
}

class Connection : public QWidget
{
    Q_OBJECT

public:
    explicit Connection(QWidget *parent = nullptr);
    ~Connection();
private slots:

    void on_btn_ok_clicked();
protected:
    void Connection::saveConfigtoIni(const QString &configName,
                                     const QString &host, quint16 port,
                                     const QString &user, const QString &pwd,
                                     bool savePwd, bool saveConfig);
     bool Connection::loadConfigFromIni();


signals:
    void sig_dbInfo(QString, QString, quint16, QString, QString, bool, bool);
    void savedConnection(const QString &configName);
private:
    Ui::Connection *ui;
    QString connection_name;
    QString host;
    quint16 port;
    QString User_Name;
    QString password;
    bool save_passwd;
    bool save_config;
};

#endif // CONNECTION_H
