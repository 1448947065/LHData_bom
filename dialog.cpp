#include "dialog.h"
#include "ui_dialog.h"

//#pragma execution_character_set("utf-8")
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    mat = nullptr;
    bom = nullptr;
    ui->cmb_sheetbom->setDisabled(true);
    ui->cmb_sheetmat->setDisabled(true);
    ui->btn_comp->setDisabled(true);
    ui->btn_refresh->setDisabled(true);
    ui->label_2->setFixedSize(110, 22);
    ui->label_5->setFixedSize(110, 22);
    ui->cmb_sheetbom->setFixedSize(160, 28);
    ui->cmb_sheetmat->setFixedSize(160, 28);
    checkManuFlag = false;
    checkDosageFlag = false;
    fillinManuFlag = false;

}

Dialog::~Dialog()
{
    delete ui;
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
    if (!bom || !mat) {
        QMessageBox::warning(this, tr("双清单生成工具"), tr("请先加载BOM和物料清单文件。"));
        return;
    }

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
                     value = rowData[8];
                     cellFormat = specialColumnFormat;
                 }
                 else if (col == 17) {
                     QXlsx::Document xlsx(matFileFullName);
                     value = xlsx.read(matchedMatRow, col);
                     resultDoc.write(matchedMatRow, col, value);
                 }
                 else if (col == 18) {
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
    matFileFullName = QFileDialog::getOpenFileName(this, tr("open file"), " ",  tr("Microsoft Excel(*.xlsx)"));

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


//void Dialog::on_ckb_dosage_check_stateChanged(int arg1)
//{
//    if (arg1 == Qt::Checked) {
//        checkDosageFlag = true;
//    } else if (arg1 == Qt::Unchecked) {
//        checkDosageFlag = false;
//    }
//}

//void Dialog::on_ckb_fillin_manu_stateChanged(int arg1)
//{
//    if (arg1 == Qt::Checked) {
//        fillinManuFlag = true;
//    } else if (arg1 == Qt::Unchecked) {
//        fillinManuFlag = false;
//    }
//}

//void Dialog::on_btn_bomcomp_openbom1_clicked()
//{
//    bomFileFullName1 = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("Microsoft Excel(*.xlsx)"));

//    bom1 = new QXlsx::Document(bomFileFullName1);
//    if (bom1) {
//        //ui->cmb_manu->setDisabled(false);

//        ui->btn_refresh->setDisabled(false);
//    }

//    if (bom1 && bom2) {
//        ui->btn_comp->setDisabled(false);
//    }
//}
