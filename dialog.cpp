#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    mat = nullptr;
    bom = nullptr;
    //QXlsx::Document xlsx;
    ui->cmb_sheetbom->setDisabled(true);
    ui->cmb_sheetmat->setDisabled(true);
    ui->btn_comp->setDisabled(true);
    ui->btn_refresh->setDisabled(true);
    ui->label_2->setFixedSize(110, 22);
    ui->label_5->setFixedSize(110, 22);
    ui->cmb_sheetbom->setFixedSize(160, 28);
    ui->cmb_sheetmat->setFixedSize(160, 28);
    //ui->lb_pathbom->setFixedSize(363, 14);
    checkManuFlag = false;
    checkDosageFlag = false;
    fillinManuFlag = false;
}
Dialog::~Dialog()
{
//    delete ui;
}
void Dialog::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    ui->label_2->setGeometry(178, 29, 110, 22);
    ui->label_5->setGeometry(178, 28, 110, 22);
    ui->cmb_sheetbom->setGeometry(294, 27, 160, 28);
    ui->cmb_sheetmat->setGeometry(294, 27, 160, 28);

    ui->lb_pathbom->move(93, 60);
    ui->lb_pathmat->move(93, 60);

}

void Dialog::on_btn_comp_clicked()
{
//    if (!bom || !mat) {
//        QMessageBox::warning(this, tr("双清单生成工具"), tr("请先加载BOM和物料清单文件。"));
//        return;
//    }

    QString selectedSheet = ui->cmb_sheetbom->currentText();
    if (selectedSheet.isEmpty()) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("请选择一个BOM工作表。"));
        return;
    }

    QXlsx::Worksheet *bomSheet = dynamic_cast<QXlsx::Worksheet*>(bom->sheet(selectedSheet));
    if (!bomSheet) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("无法加载选中的BOM工作表。"));
        return;
    }

    // 获取 BOM 表中的制造商列
    int bomManufacturerCol = -1;
    for (int col = 1; col <= bomSheet->dimension().columnCount(); ++col) {
        QString header = bomSheet->read(1, col).toString().trimmed();
        if (header.contains("制造商 ")) {
            bomManufacturerCol = col;
            break;
        }
    }

    if (bomManufacturerCol == -1) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("BOM 表中未找到制造商列。"));
        return;
    }

    QSet<QString> bomPartNumbers;
    int startRow = 2;
    while (bomSheet->read(startRow, 1).toString().trimmed().isEmpty() && startRow <= bomSheet->dimension().rowCount()) {
        startRow++;
    }
    int bomRowCount = bomSheet->dimension().rowCount();
    for (int row = startRow; row <= bomRowCount; ++row) {
        QString partNumber = bomSheet->read(row, 3).toString().trimmed();
        if (!partNumber.isEmpty()) {
            bomPartNumbers.insert(partNumber);
        }
    }

    QString matSheetName = mat->sheetNames().first();
    QXlsx::Worksheet *matSheet = dynamic_cast<QXlsx::Worksheet*>(mat->sheet(matSheetName));
    if (!matSheet) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("加载物料清单表失败。"));
        return;
    }

    // 找出物料清单表中的生产厂商列
    int producerCol = -1;
    for (int col = 1; col <= matSheet->dimension().columnCount(); ++col) {
        QString header = matSheet->read(1, col).toString().trimmed();
        if (header.contains("生产厂商")) {
            producerCol = col;
            break;
        }
    }

    if (producerCol == -1) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("物料清单表中未找到生产厂商列。"));
        return;
    }

    QXlsx::Document resultDoc;
    int resultRow = 1;
    QVector<QVector<QVariant>> matchedRows;
    QSet<QString> matchedPartNumbers;

    int matRowCount = matSheet->dimension().rowCount();
    int matColCount = matSheet->dimension().columnCount();

    // 收集匹配和不匹配的行数据
    QHash<QString, QVector<QVariant>> matData; // key: 料号, value: 整行数据
    for (int row = 2; row <= matRowCount; ++row) {
        QString partNumber = matSheet->read(row, 2).toString().trimmed();
        if (!partNumber.isEmpty()) {
            QVector<QVariant> rowData;
            for (int col = 1; col <= matColCount; ++col) {
                rowData.append(matSheet->read(row, col));
            }
            matData.insert(partNumber, rowData);
        }
    }

    QXlsx::Format headerFormat;
    headerFormat.setFontName("宋体");
    headerFormat.setFontSize(12);
    headerFormat.setFontBold(true);
    headerFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    headerFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    headerFormat.setBorderStyle(QXlsx::Format::BorderThin);

    // 写入标题行
    for (int col = 1; col <= matColCount; ++col) {
        resultDoc.write(resultRow, col, matSheet->read(1, col), headerFormat);
    }
    resultDoc.setRowHeight(resultRow, 30.0);
    resultRow++;

    QXlsx::Format specialHeaderFormat;
    specialHeaderFormat.setFontName("微软雅黑");
    specialHeaderFormat.setFontSize(14);
    specialHeaderFormat.setFontBold(true);
    specialHeaderFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    specialHeaderFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    specialHeaderFormat.setBorderStyle(QXlsx::Format::BorderThin);

    for (int col = 2; col <= 5; ++col) {
        resultDoc.write(1, col, matSheet->read(1, col), specialHeaderFormat);
    }

    QXlsx::Format leftAlignFormat;
    leftAlignFormat.setFontName("宋体");
    leftAlignFormat.setFontSize(10);
    leftAlignFormat.setHorizontalAlignment(QXlsx::Format::AlignLeft);
    leftAlignFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    leftAlignFormat.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format rightAlignFormat;
    rightAlignFormat.setFontName("宋体");
    rightAlignFormat.setFontSize(10);
    rightAlignFormat.setHorizontalAlignment(QXlsx::Format::AlignRight);
    rightAlignFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    rightAlignFormat.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format specialLeftAlignFormat;
    specialLeftAlignFormat.setFontName("微软雅黑");
    specialLeftAlignFormat.setFontSize(10);
    specialLeftAlignFormat.setHorizontalAlignment(QXlsx::Format::AlignLeft);
    specialLeftAlignFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    specialLeftAlignFormat.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format specialColumnFormat;
    specialColumnFormat.setFontName("微软雅黑");
    specialColumnFormat.setFontSize(10);
    specialColumnFormat.setHorizontalAlignment(QXlsx::Format::AlignLeft);
    specialColumnFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    specialColumnFormat.setBorderStyle(QXlsx::Format::BorderThin);

    QSet<QString> matPartNumbers;
    for (int row = 2; row <= matRowCount; ++row) {
        QString partNumber = matSheet->read(row, 2).toString().trimmed();
        if (!partNumber.isEmpty()) {
            matPartNumbers.insert(partNumber);
        }
    }

    int serialNumber = 1;
    for (int bomRow = startRow; bomRow <= bomRowCount; ++bomRow) {
        QString partNumber = bomSheet->read(bomRow, 3).toString().trimmed();
        if (partNumber.isEmpty()) continue;

        // 查找物料清单表中匹配的行
        int matchedMatRow = -1;
        for (int matRow = 2; matRow <= matRowCount; ++matRow) {
            if (matSheet->read(matRow, 2).toString().trimmed() == partNumber) {
                matchedMatRow = matRow;
                break;
            }
        }

     // 处理每一列
     for (int col = 1; col <= matColCount; ++col) {
         QVariant value;
         QXlsx::Format cellFormat = specialColumnFormat;

         if (col == 1) {
             // 序号列
             value = serialNumber;
             cellFormat = rightAlignFormat;
         }
         else if (matchedMatRow != -1) {
             // 匹配的行 - 完全按照您注释中的原始逻辑处理
             const QVector<QVariant>& rowData = matData[partNumber];

             if (col >= 2 && col <= 5) {
                 value = rowData[col-1];
                 cellFormat = specialLeftAlignFormat;
             }
             else if (col == producerCol) {
                 value = rowData[producerCol-1];
                 cellFormat = specialLeftAlignFormat;
             }
             else if (col == 9) {
                     value = rowData[8]; // 第9列（索引8）
                 cellFormat = specialColumnFormat;
             }
             else if (col == 17) {
                 // 复制匹配行的17、18列内容
                 QXlsx::Document xlsx(matFileFullName);
                 value = xlsx.read(matchedMatRow, col);
                 resultDoc.write(matchedMatRow, col, value);
             }
             else {
                 value = rowData[col-1];
                 cellFormat = specialColumnFormat;
             }
         }
         else {
             // 未匹配的行 - 只保留关键信息
             if (col == 2) value = partNumber;
             else if (col == 3) value = bomSheet->read(bomRow, 2); // 名称
             else if (col == 4) value = bomSheet->read(bomRow, 4); // 规格型号
             else if (col == 5) value = bomSheet->read(bomRow, 9);
             // 其他列留空
         }

         resultDoc.write(resultRow, col, value, cellFormat);
     }

     resultDoc.setRowHeight(resultRow, 15.0);
     resultRow++;
     serialNumber++;
 }

    // 设置列宽
    for (int col = 1; col <= matColCount; ++col) {
        resultDoc.setColumnWidth(col, 15.0);
    }

    resultDoc.setColumnWidth(2, 22.0);
    resultDoc.setColumnWidth(4, 85.0);
    resultDoc.setColumnWidth(5, 30.0);
    resultDoc.setColumnWidth(6, 25.0);
    resultDoc.setColumnWidth(8, 50.0);
    resultDoc.setColumnWidth(9, 33.0);

    for (int i = 13; i <= 18; ++i) {
        resultDoc.setColumnWidth(i, 25.0);
    }

    QString bomFilePath = bomFileFullName.section("/", 0, -2);
    QString bomFileName = bomFileFullName.section("/", -1, -1).section(".", 0, -2);
    QString bomFileExt = bomFileFullName.section(".", -1, -1);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString saveFileName = bomFilePath + "/" + bomFileName + "-双清单-" + timestamp + "." + bomFileExt;

    resultDoc.saveAs(saveFileName);
    QMessageBox::information(this, tr("双清单生成工具"), tr("文件已成功保存到: ") + saveFileName);
}

void Dialog::onLayoutAdjusted() {

    QResizeEvent event(this->size(), this->size());
    QCoreApplication::sendEvent(this, &event);

}
void Dialog::on_btn_openmat_clicked()
{
    matFileFullName = QFileDialog::getOpenFileName(this, tr("open file"), " ",  tr("Microsoft Excel(*.xlsx *.xls)"));

    ui->lb_pathmat->setText(matFileFullName);
    mat = new QXlsx::Document(matFileFullName);
    if (mat) {
        ui->cmb_sheetmat->setDisabled(false);
        ui->cmb_sheetmat->clear();
        ui->cmb_sheetmat->addItems(mat->sheetNames());
        ui->btn_refresh->setDisabled(false);
    }

    if (mat && bom) {
        ui->btn_comp->setDisabled(false);
    }

    //adjustWindowSize(); // 调整窗口大小
}

void Dialog::on_btn_openbom_clicked()
{
    bomFileFullName = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("Microsoft Excel(*.xlsx)"));
    ui->lb_pathbom->setText(bomFileFullName);
    bom = new QXlsx::Document(bomFileFullName);
    if (bom) {
        //ui->cmb_manu->setDisabled(false);
        ui->cmb_sheetbom->setDisabled(false);
        //ui->ckb_manu_check->setDisabled(false);
        ui->cmb_sheetbom->clear();
        ui->cmb_sheetbom->addItems(bom->sheetNames());
        ui->btn_refresh->setDisabled(false);
    }

    if (bom && mat) {
        ui->btn_comp->setDisabled(false);
    }

    //adjustWindowSize(); // 调整窗口大小
}

void Dialog::on_btn_refresh_clicked()
{

    free(bom);
    free(mat);
     ui->lb_pathbom->setText("");
     ui->lb_pathmat->setText("");
     ui->cmb_sheetbom->clear();
     ui->cmb_sheetmat->clear();

    if (!bomFileFullName.isEmpty()) {
        bom = new QXlsx::Document(bomFileFullName);
    }
    if (!matFileFullName.isEmpty()) {
        mat = new QXlsx::Document(matFileFullName);
    }

    QMessageBox::information(this, "BOM & Material Reload", "文件已重新加载");
}



void Dialog::on_btn_str_clicked()
{
    if (!bom || !mat) {
        QMessageBox::warning(this, tr("BOM匹配工具"), tr("请先加载BOM与主库文件。"));
        return;
    }
    const QString bomSheetName = ui->cmb_sheetbom->currentText();
    const QString matSheetName = ui->cmb_sheetmat->currentText();
    if (bomSheetName.isEmpty() || matSheetName.isEmpty()) {
        QMessageBox::warning(this, tr("BOM匹配工具"), tr("请先选择BOM与主库的工作表。"));
        return;
    }

    auto *bomSheet = dynamic_cast<QXlsx::Worksheet*>(bom->sheet(bomSheetName));
    auto *matSheet = dynamic_cast<QXlsx::Worksheet*>(mat->sheet(matSheetName));
    if (!bomSheet || !matSheet) {
        QMessageBox::warning(this, tr("BOM匹配工具"), tr("工作表加载失败。"));
        return;
    }

    auto norm = [](QString s)->QString {
        s = s.trimmed().toLower();
        s.replace(QChar(0x3000), QChar(' '));
        s.remove(QRegularExpression(u8R"([（(].*?[)）])"));
        s.replace("\n", " ").replace("\r", " ").replace("\t", " ");
        s.remove(QRegularExpression(u8R"([：:、/,，;；\-\|])"));
        s.remove(' ');
        return s;
    };

    auto detectHeaderRow = [](QXlsx::Worksheet *ws, int headerScanRows = 5)->int {
        if (!ws) return 1;
        const int rows = qMin(headerScanRows, ws->dimension().rowCount());
        const int cols = ws->dimension().columnCount();
        int bestRow = 1, bestCnt = -1;
        for (int r = 1; r <= rows; ++r) {
            int cnt = 0;
            for (int c = 1; c <= cols; ++c) {
                if (!ws->read(r, c).toString().trimmed().isEmpty()) ++cnt;
            }
            if (cnt > bestCnt) { bestCnt = cnt; bestRow = r; }
        }
        return bestRow;
    };

    auto findColRobust = [&](QXlsx::Worksheet *ws,
                             const QStringList &candidates,
                             int headerRow)->int {
        if (!ws) return -1;
        const int cols = ws->dimension().columnCount();

        QStringList cand;
        for (const auto &k : candidates) cand << norm(k);

        for (int c = 1; c <= cols; ++c) {
            const QString raw = ws->read(headerRow, c).toString();
            if (raw.trimmed().isEmpty()) continue;
            const QString h = norm(raw);

            for (const QString &k : cand) {
                if (h == k || h.contains(k)) {
                    return c;
                }
            }
        }
        return -1;
    };

    const int bomHeaderRow = detectHeaderRow(bomSheet, 5);
    const int matHeaderRow = detectHeaderRow(matSheet, 5);

    const QStringList BOM_REF_ALIASES = {
        "位号","位 号","参考位号","refdes","reference","designation"
    };
    const QStringList BOM_NAME_ALIASES = {
        "名称","元器件名称","器件名称","name","description","desc","中文名称"
    };
    const QStringList PART_ALIASES = {
        "料号","物料号","mpn","part","partno","partnumber","mfgpn","制造商料号"
    };
    const QStringList SPEC_ALIASES = {
        "规格","规格型号","规格型号描述","参数","spec","specification","description","desc"
    };
    const QStringList POWER_ALIASES = {
        "功耗","功率","power","powerconsumption"
    };
    const QStringList HEIGHT_ALIASES = {
        "封装高度","高度","packageheight","height","封装厚度","厚度"
    };

    const int bomRefCol  = findColRobust(bomSheet, BOM_REF_ALIASES,  bomHeaderRow);
    const int bomNameCol = findColRobust(bomSheet, BOM_NAME_ALIASES, bomHeaderRow);
    const int bomPartCol = findColRobust(bomSheet, PART_ALIASES,     bomHeaderRow);

    const int matPartCol   = findColRobust(matSheet, PART_ALIASES,   matHeaderRow);
    const int matNameCol   = findColRobust(matSheet, BOM_NAME_ALIASES, matHeaderRow); // 备用
    const int matSpecCol   = findColRobust(matSheet, SPEC_ALIASES,   matHeaderRow);
    const int matPowerCol  = findColRobust(matSheet, POWER_ALIASES,  matHeaderRow);
    const int matHeightCol = findColRobust(matSheet, HEIGHT_ALIASES, matHeaderRow);

    auto collectHeaderRowText = [&](QXlsx::Worksheet *ws, int headerRow)->QStringList {
        QStringList items;
        const int cols = ws->dimension().columnCount();
        for (int c = 1; c <= cols; ++c) {
            const QString t = ws->read(headerRow, c).toString();
            if (!t.trimmed().isEmpty()) items << QString("[%1]%2").arg(c).arg(t.trimmed());
        }
        return items;
    };

    if (bomRefCol == -1 || bomPartCol == -1) {
        const QStringList heads = collectHeaderRowText(bomSheet, bomHeaderRow);
        QMessageBox::warning(
            this, tr("BOM匹配工具"),
            tr("BOM表头中未找到必需的“位号/料号”列。\n"
               "检测的表头行：第 %1 行\n"
               "实际读取到的表头：\n%2")
                .arg(bomHeaderRow)
                .arg(heads.join('\n'))
        );
        return;
    }
    if (matPartCol == -1 || matSpecCol == -1 || matPowerCol == -1 || matHeightCol == -1) {
        const QStringList heads = collectHeaderRowText(matSheet, matHeaderRow);
        QMessageBox::warning(
            this, tr("BOM匹配工具"),
            tr("主库表头缺少必要列（料号/规格/功耗/封装高度）。\n"
               "检测的表头行：第 %1 行\n"
               "实际读取到的表头：\n%2")
                .arg(matHeaderRow)
                .arg(heads.join('\n'))
        );
        return;
    }

    struct MasterRow { QString name, spec, power, height; };
    QHash<QString, MasterRow> master;
    const int matRows = matSheet->dimension().rowCount();
    for (int r = matHeaderRow + 1; r <= matRows; ++r) {
        const QString part = matSheet->read(r, matPartCol).toString().trimmed();
        if (part.isEmpty()) continue;
        MasterRow m;
        if (matNameCol != -1) m.name = matSheet->read(r, matNameCol).toString().trimmed();
        m.spec   = matSheet->read(r, matSpecCol).toString().trimmed();
        m.power  = matSheet->read(r, matPowerCol).toString().trimmed();
        m.height = matSheet->read(r, matHeightCol).toString().trimmed();
        master.insert(part, m);
    }

    QXlsx::Document out;

    out.write(1, 1, "位号");
    out.write(1, 2, "元器件名称");
    out.write(1, 3, "料号");
    out.write(1, 4, "规格");
    out.write(1, 5, "功耗");
    out.write(1, 6, "封装高度");

    out.setColumnWidth(1, 18.0);
    out.setColumnWidth(2, 28.0);
    out.setColumnWidth(3, 22.0);
    out.setColumnWidth(4, 42.0);
    out.setColumnWidth(5, 14.0);
    out.setColumnWidth(6, 16.0);

    int outRow = 2;
    const int bomRows = bomSheet->dimension().rowCount();
    for (int r = bomHeaderRow + 1; r <= bomRows; ++r) {
        const QString ref  = bomSheet->read(r, bomRefCol).toString().trimmed();
        const QString part = bomSheet->read(r, bomPartCol).toString().trimmed();
        if (part.isEmpty()) continue;

        QString name;
        if (bomNameCol != -1) name = bomSheet->read(r, bomNameCol).toString().trimmed();

        auto it = master.constFind(part);
        if (it == master.constEnd()) continue;

        const QString finalName = name.isEmpty() ? it->name : name;

        out.write(outRow, 1, ref);
        out.write(outRow, 2, finalName);
        out.write(outRow, 3, part);
        out.write(outRow, 4, it->spec);
        out.write(outRow, 5, it->power);
        out.write(outRow, 6, it->height);
        ++outRow;
    }

    const QString basePath = bomFileFullName.section("/", 0, -2);
    const QString baseName = bomFileFullName.section("/", -1).section(".", 0, -2);
    const QString ext      = "xlsx";
    const QString ts       = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    const QString outPath  = basePath + "/" + baseName + "-匹配结果-" + ts + "." + ext;

    if (out.saveAs(outPath)) {
        QMessageBox::information(this, tr("BOM匹配工具"), tr("已生成文件：\n%1").arg(outPath));
    } else {
        QMessageBox::warning(this, tr("BOM匹配工具"), tr("保存失败，请检查路径权限。"));
    }
}
