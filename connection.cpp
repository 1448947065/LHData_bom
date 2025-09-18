#include "connection.h"
#include "ui_connection.h"

Connection::Connection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Connection)
{
    ui->setupUi(this);
    ui->line_port->setPlaceholderText("3306");
    loadConfigFromIni();
}

Connection::~Connection()
{
    delete ui;
}

void Connection::on_btn_ok_clicked()
{
    connection_name = ui->line_con_name->text();
    host = ui->line_host->text();
    port = ui->line_port->text().toInt();
    User_Name = ui->line_user_name->text();
    password = ui->line_password->text();
    save_passwd = ui->check_passwd->isChecked();
    save_config = ui->check_con->isChecked();
    qDebug()<<connection_name<<host<<port<<User_Name<<password<<save_passwd<<save_config;
    qDebug() << "=== 信号发射前 ===";
    if (save_config) {
        saveConfigtoIni(connection_name,host, port,
                        User_Name, password,save_passwd, save_config);
    }
    emit sig_dbInfo(connection_name,host, port,
                    User_Name, password, save_passwd, save_config);
    qDebug() << "=== 信号发射后 ===";
    qApp->processEvents();  // 强制立即处理
}

void Connection::saveConfigtoIni(const QString &configName,
                                 const QString &host, quint16 port,
                                 const QString &user, const QString &pwd,
                                 bool savePwd, bool saveConfig)
{
    QSettings settings("config.ini", QSettings::IniFormat);

    settings.beginGroup(configName);
    settings.setValue("host", host);
    settings.setValue("port", port);
    settings.setValue("user", user);

    if (savePwd) {
        settings.setValue("password", pwd);
    } else {
        settings.remove("password");
    }

    settings.setValue("savePwd", savePwd);
    settings.setValue("saveConfig", saveConfig);
    settings.endGroup();

    qDebug() << "配置已保存到config.ini";
}

bool Connection::loadConfigFromIni()
{
    QSettings settings("config.ini", QSettings::IniFormat);

    // 检查配置文件是否存在
    if (!QFileInfo("config.ini").exists()) {
        qDebug() << "配置文件不存在";
        return false;
    }

    // 直接读取 [lhlist] 分组
    settings.beginGroup("lhlist");

    // 检查必要配置项是否存在
    if (!settings.contains("host")) {
        qDebug() << "lhlist 配置不完整";
        settings.endGroup();
        return false;
    }

    // 读取配置值
    QString host = settings.value("host").toString();
    quint16 port = settings.value("port").toUInt();
    QString user = settings.value("user").toString();
    bool savePwd = settings.value("savePwd").toBool();
    QString pwd = savePwd ? settings.value("password").toString() : "";
    bool saveConfig = settings.value("saveConfig").toBool();

    settings.endGroup();

    // 自动填充到UI控件
    ui->line_con_name->setText("lhlist");  // 固定连接名称
    ui->line_host->setText(host);
    ui->line_port->setText(QString::number(port));
    ui->line_user_name->setText(user);
    ui->line_password->setText(pwd);
    ui->check_passwd->setChecked(savePwd);
    ui->check_con->setChecked(saveConfig);

    qDebug() << "配置已加载到UI";
    return true;
}
