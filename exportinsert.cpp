#include "exportinsert.h"
#include "ui_exportinsert.h"

ExportInsert::ExportInsert(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportInsert)
{
    ui->setupUi(this);
    connect(this, &ExportInsert::mysqlCreate, dbManagerExport, &MySql::initializeDatabase);
    connect(dbManagerExport, &MySql::progressChanged,
            this, [=](int cur, int total, bool finished, bool ok, const QString &msg){
        if (!finished) {
            ui->progressBar->setRange(0, total);
            ui->progressBar->setValue(cur);
        } else {
            ui->progressBar->setRange(0, 100);
            ui->progressBar->setValue(100);
            QMessageBox::information(this, ok ? "导入完成" : "导入失败", msg);
        }
    });
    ui->progressBar->setValue(0);
}
ExportInsert::~ExportInsert()
{
    delete ui;
}

void ExportInsert::on_pushButton_clicked()
{
    filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择文件"),
        QDir::homePath(),
        tr("Excel 文件 (*.xlsx);;所有文件 (*.*)")
    );
    if (filePath.isEmpty()) return;

}

DbConfig ExportInsert::readDbConfig(const QString &connName)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(connName);
//    const QStringList groups = settings.childGroups();
//    if (!groups.contains(connName)) {
//        qDebug() << "未找到连接名" << connName << "在配置文件中！";
//        return DbConfig();
//    }

    DbConfig cfg;
    cfg.host = settings.value("host", "localhost").toString();
    cfg.port = settings.value("port", 3306).toInt();
    cfg.user = settings.value("user").toString();
    cfg.password = settings.value("password").toString();
    cfg.savePwd = settings.value("savePwd", false).toBool();
    cfg.saveConfig = settings.value("saveConfig", false).toBool();

    settings.endGroup();
    return cfg;
}
void ExportInsert::updateProcess()
{

}

void ExportInsert::on_buttonBox_accepted()
{

    QXlsx::Document bomXlsxR(filePath);
    int maxCol = bomXlsxR.dimension().lastColumn();
    int maxRow = bomXlsxR.dimension().lastRow();

    QVector<QVector<QString>> columnData(maxCol);
    for (int i = 0; i < maxCol; i++) {
        columnData[i].resize(maxRow);
    }

    for (int row = 1; row <= maxRow; ++row) {
        for (int col = 1; col <= maxCol; ++col) {
            QXlsx::Cell* cell = bomXlsxR.cellAt(row, col);
            QString value = cell ? cell->value().toString() : "";
            columnData[col-1][row-1] = value;
        }
    }

    // 从界面输入框里拿连接名（比如 "lhlist" 或 "rk3588"）
    QString connName = ui->line_cnName->text().trimmed();
    QString dbName   = ui->line_dbName->text().trimmed();
    // 从 ini 里读配置
    DbConfig cfg = readDbConfig(connName);
    if (cfg.user.isEmpty()) {
        qDebug() << "错误：config.ini 中没有找到连接名" << connName;
        return;
    }

    // 发信号出去
    emit mysqlCreate(
        connName,
        dbName,
        cfg.host,
        cfg.user,
        cfg.password,
        columnData
    );

    qDebug() << "配置读取成功:"
             << "host=" << cfg.host
             << "port=" << cfg.port
             << "user=" << cfg.user
             << "savePwd=" << cfg.savePwd
             << "saveConfig=" << cfg.saveConfig;
}

