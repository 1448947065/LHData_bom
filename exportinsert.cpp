#include "exportinsert.h"
#include "ui_exportinsert.h"

ExportInsert::ExportInsert(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportInsert)
{
    ui->setupUi(this);
    connect(this, &ExportInsert::mysqlCreate, dbManagerExport, &MySql::initializeDatabase);
    connect(dbManagerExport, &MySql::progressChanged,
            this, [=](int cur, int total, bool finished, bool ok, const QString &msg) {
        if (!finished) {
            ui->progressBar->setRange(0, total);
            ui->progressBar->setValue(cur);
        } else {

            ui->progressBar->setRange(0, 100);
            ui->progressBar->setValue(100);

            QMessageBox::information(this, ok ? "导入完成" : "导入失败", msg);
            ui->progressBar->setValue(0);

            m_running = false;
            finished = true;
        }
    });
    applyGlobalStyle();
    ui->progressBar->setValue(0);
    m_running = false;
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
void ExportInsert::getDbmes(QString connName, QString dbName)
{
    e_connName = connName;
    e_dbName   = dbName;
    qDebug()<<"1234"<<connName;
}



void ExportInsert::on_btn_ok_clicked()
{
    if (m_running) return;
    m_running = true;

    ui->progressBar->setRange(0, 0);

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

    DbConfig cfg = readDbConfig(e_connName);
    if (cfg.user.isEmpty()) {
        qDebug() << "错误：config.ini 中没有找到连接名" << e_connName;
        m_running = false;  // 导入操作失败时，重置标志位
        return;
    }

    qDebug() << "准备发射 mysqlCreate 信号";
    emit mysqlCreate(
        e_connName,
        e_dbName,
        cfg.host,
        cfg.user,
        cfg.password,
        columnData
    );
    qDebug() << "mysqlCreate 信号已发射";

    // 这行不再立即关闭对话框，改为只设置为已完成状态
    this->setResult(QDialog::Rejected);

    // 完成后启用 OK 按钮，并重置标志位
    m_running = false;  // 重置操作状态

    qDebug() << "配置读取成功:"
             << "host=" << cfg.host
             << "port=" << cfg.port
             << "user=" << cfg.user
             << "savePwd=" << cfg.savePwd
             << "saveConfig=" << cfg.saveConfig;

}

void ExportInsert::applyGlobalStyle()
{
    // 主窗口样式
    this->setStyleSheet(R"(
        /* 基础窗口样式 */
        QDialog {
            background-color: #f5f7fa;
            font-family: "Microsoft YaHei";
        }

        /* 所有按钮统一蓝色风格 */
        QPushButton {
            background-color: #4a9ff9;
            color: white;
            border-radius: 4px;
            padding: 6px 12px;
            min-width: 80px;
            border: none;
        }
        QPushButton:hover {
            background-color: #3a8ee6;
        }
        QPushButton:pressed {
            background-color: #2a7dd6;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #888888;
        }

        /* 蓝色进度条 */
        QProgressBar {
            border: 1px solid #d1d5db;
            border-radius: 4px;
            background-color: white;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #4a9ff9;
            border-radius: 3px;
        }
    )");
}
