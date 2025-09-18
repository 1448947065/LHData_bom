#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitUi();
    test();
    initiazeTreeView();
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::InitUi()
{
    ui->btn_sql_mar->setText(tr("数据库"));
    ui->btn_bom->setText(tr("双清单管理工具"));
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("background: transparent;");


    QIcon qicon;
    qicon.addFile(":/img/coc.png");
    ui->btn_connection->setIcon(qicon);
    ui->btn_connection->setIconSize(QSize(64,64));
    ui->btn_connection->setFixedSize(50, 50);

    QIcon qicon1;
    qicon1.addFile(":/img/sec.png");
    ui->btn_search->setIcon(qicon1);
    ui->btn_search->setIconSize(QSize(40,40));
    ui->btn_search->setFixedSize(50, 50);

    auto *root = new QWidget(this);
    root->setAttribute(Qt::WA_StyledBackground, true);
    root->setStyleSheet("background: transparent;");

    auto *rootLay = new QVBoxLayout(root);
    rootLay->setContentsMargins(16, 16, 16, 16);
    rootLay->setSpacing(0);

    auto *card = new QWidget(root);
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet("#card{ background:white; border-radius:12px; }");

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 70));
    card->setGraphicsEffect(shadow);

    rootLay->addWidget(card);

    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(0, 0, 0, 0);
    cardLay->setSpacing(0);

    auto *title = new TitleBar(card);
    cardLay->addWidget(title);
#if global_menu
    auto *menu = new QMenuBar(card);
    menu->setAttribute(Qt::WA_StyledBackground, true);
    menu->setStyleSheet(
        "QMenuBar{background:white; color:black; border:none;}"
        "QMenuBar::item{background:transparnt; padding:4px 10px;}"
        "QMenuBar::item:selected{background:#eaeaea;}"
    );
    menu->addMenu(tr("文件"))->addAction("打开");
    menu->addMenu(tr("编辑"));
    cardLay->addWidget(menu);
#endif
    ui->centralwidget->setParent(card);
    ui->centralwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cardLay->addWidget(ui->centralwidget);

    setCentralWidget(root);

    connect(title, &TitleBar::minimizeClicked, this, &MainWindow::showMinimized);
    connect(title, &TitleBar::maximizeClicked, [this, title]{
        isMaximized() ? showNormal() : showMaximized();
        title->syncMaxButton(isMaximized());
    });
    connect(title, &TitleBar::closeClicked, this, &MainWindow::close);
    connect(this, &MainWindow::mysqlCreate, dbManager, &MySql::initializeDatabase);
    connect(con, &Connection::sig_dbInfo, dbManager, &MySql::handleDbInfo);
}
void MainWindow::test()
{


//    QXlsx::Document bomXlsxR("D:/work project/sql/untitled1/不合格BOM清单.xlsx");
//    int maxCol = bomXlsxR.dimension().lastColumn();
//    int maxRow = bomXlsxR.dimension().lastRow();
//    qDebug()<<maxRow;

//    QVector<QVector<QString>> columnData(maxCol);
//    for (int i = 0; i < maxCol; i++) {
//        columnData[i].resize(maxRow); //预分配每列的空间
//    }
//        qDebug() << "表格大小：" << maxRow << "行," << maxCol << "列";

//        for (int row = 1; row <= maxRow; ++row) {
//            for (int col = 1; col <= maxCol; ++col) {
//                QXlsx::Cell* cell = bomXlsxR.cellAt(row, col);
//                QString value = cell ? cell->value().toString() : "";
//                columnData[col-1][row-1] = value;
//            }
//        }
//        emit mysqlCreate(columnData);

//        for (int col = 0; col < maxCol; ++col) {
//            qDebug() << "列" << col+1 << "数据:";
//            for (const QString& value : columnData[col]) {
//                qDebug() << value;
//            }
//            qDebug() << "---------------------";
//        }
}

void MainWindow::ConnectSlot()
{

}

void MainWindow::on_btn_sql_mar_clicked()
{
    auto *m = new MySql(this);
    ui->stackedWidget->insertWidget(1, m);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_btn_bom_clicked()
{
//    auto *b = new lhxlsx(this);
//    ui->stackedWidget->insertWidget(0, b);
//    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::insert()
{

}

void MainWindow::on_btn_connection_clicked()
{
    con->show();
}

void MainWindow::on_btn_search_clicked()
{
    QSqlTableModel *model = dbManager->getModel("lhbom");
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}
void MainWindow::initiazeTreeView()
{
    qDebug() << "======== 初始化树状视图 ========";

    // 重置模型
    if (treeModel) {
        delete treeModel;
    }
    treeModel = new QStandardItemModel(this);
    ui->treeView->setModel(treeModel);

    // 强制显示和激活
    ui->treeView->show();
    ui->treeView->activateWindow();
    ui->treeView->raise();
    ui->treeView->setFocus();

    // 基本设置
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionBehavior(QTreeView::SelectRows);
    ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
    ui->treeView->setIndentation(10);

    // 样式设置（精简版）
    ui->treeView->setStyleSheet(
        "QTreeView { background: white; border: none; }"
        "QTreeView::item { height: 35px; padding-left: 15px; }"
    );

    // 创建根节点
    QStandardItem *rootItem = new QStandardItem(QIcon(":/img/co.png"), "数据库连接");
    treeModel->appendRow(rootItem);

    // 加载连接
    loadSavedConnections(rootItem);

    // 信号连接（增强调试）
    bool conn1 = connect(ui->treeView, &QTreeView::doubleClicked,
                        this, &MainWindow::onConnectionDoubleClicked,
                        Qt::UniqueConnection);

    bool conn2 = connect(ui->treeView, &QTreeView::clicked,
                        [](const QModelIndex &idx){ qDebug() << "单击事件:" << idx; });

    qDebug() << "信号连接状态 - 双击:" << conn1 << "单击:" << conn2;

    // 安装事件过滤器
    ui->treeView->installEventFilter(this);
    qDebug() << "树状视图初始化完成 - 可见:" << ui->treeView->isVisible()
             << "激活:" << ui->treeView->isActiveWindow();
}

void MainWindow::loadSavedConnections(QStandardItem *parentItem)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (const QString &connName, groups) {
        settings.beginGroup(connName);

        // 使用默认连接图标（未连接状态）
        QStandardItem *connItem = new QStandardItem(QIcon(":/img/connection.png"), connName);
        connItem->setData(connName, Qt::UserRole + 1);
        connItem->setData("connection", Qt::UserRole + 2);

        // 添加连接信息提示（显示未连接状态）
        QString tooltip = QString("主机: %1\n端口: %2\n用户: %3\n状态: 未连接")
                            .arg(settings.value("host").toString())
                            .arg(settings.value("port").toString())
                            .arg(settings.value("user").toString());
        connItem->setToolTip(tooltip);

        parentItem->appendRow(connItem);
        settings.endGroup();
    }

    // 如果没有连接，显示提示信息
    if (groups.isEmpty()) {
        QStandardItem *emptyItem = new QStandardItem("暂无连接配置");
        emptyItem->setEnabled(false);
        emptyItem->setSelectable(false);
        parentItem->appendRow(emptyItem);
    }
}
void MainWindow::onConnectionDoubleClicked(const QModelIndex &index)
{
    QStandardItem *item = treeModel->itemFromIndex(index);

    // 只处理连接项（不是根节点或其他项）
    if (!item || !item->parent() || item->parent()->text() != "数据库连接") {
        return;
    }

    QString connName = item->text();
    qDebug() << "双击连接项:" << connName;

    // 检查是否已经展开（有子项）
    if (item->rowCount() > 0) {
        // 如果已经有子项，只是切换展开/折叠状态
        if (ui->treeView->isExpanded(index)) {
            ui->treeView->collapse(index);
            qDebug() << "折叠连接项:" << connName;
        } else {
            ui->treeView->expand(index);
            qDebug() << "展开连接项:" << connName;
        }
    } else {
        // 如果没有子项，说明是第一次点击，需要加载数据库
        qDebug() << "首次连接:" << connName;

        // 执行数据库连接并加载数据库列表
        connectToDatabase(connName);
    }
}

void MainWindow::connectToDatabase(const QString &connName)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(connName);

    QString host = settings.value("host").toString();
    quint16 port = settings.value("port").toUInt();
    QString user = settings.value("user").toString();
    QString pwd = settings.value("password").toString();

    // 创建数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(pwd);

    if (!db.open()) {
        QMessageBox::critical(this, "连接失败",
                            QString("无法连接到数据库:\n%1").arg(db.lastError().text()));
        return;
    }

    // 连接成功，更新UI状态
    QStandardItem *connItem = findConnectionItem(connName);
    if (connItem) {
        connItem->setIcon(QIcon(":/img/connection_active.png"));

        // 更新工具提示显示已连接状态
        QString newTooltip = connItem->toolTip().replace("状态: 未连接", "状态: 已连接 ✓");
        connItem->setToolTip(newTooltip);

        // 关键修复：检查是否已经加载过数据库，避免重复添加
        if (connItem->rowCount() == 0) { // 只有当没有子项时才加载
            // 加载所有数据库（根据图片显示有多个数据库）
            QSqlQuery query(db);
            if (query.exec("SHOW DATABASES")) {
                while (query.next()) {
                    QString dbName = query.value(0).toString();

                    // 创建数据库项
                    QStandardItem *dbItem = new QStandardItem(QIcon(":/img/database.png"), dbName);
                    dbItem->setData(dbName, Qt::UserRole + 1);
                    dbItem->setData("database", Qt::UserRole + 2);

                    connItem->appendRow(dbItem);

                    // 如果是lhlist数据库，预加载其下的所有表
                    if (dbName == "lhlist") {
                        QSqlQuery tableQuery(db);
                        if (tableQuery.exec(QString("SHOW TABLES FROM `%1`").arg(dbName))) {
                            while (tableQuery.next()) {
                                QString tableName = tableQuery.value(0).toString();

                                // 创建表项
                                QStandardItem *tableItem = new QStandardItem(QIcon(":/img/table.png"), tableName);
                                tableItem->setData(tableName, Qt::UserRole + 1);
                                tableItem->setData("table", Qt::UserRole + 2);

                                dbItem->appendRow(tableItem);
                            }
                        }
                    }
                }
            }
        }

        // 展开连接项
        ui->treeView->expand(connItem->index());
    }

    qDebug() << "成功连接到:" << connName;
    settings.endGroup();
}
QStandardItem* MainWindow::findConnectionItem(const QString &connName)
{
    QStandardItem *root = treeModel->item(0);
    if (!root) return nullptr;

    for (int i = 0; i < root->rowCount(); ++i) {
        QStandardItem *item = root->child(i);
        if (item && item->text() == connName) {
            return item;
        }
    }
    return nullptr;
}

// 右键菜单处理
void MainWindow::onTreeViewContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    QStandardItem *item = treeModel->itemFromIndex(index);

    QMenu contextMenu(this);

    // 设置菜单样式，避免黑色背景
    contextMenu.setStyleSheet(
        "QMenu {"
        "    background-color: white;"
        "    border: 1px solid #cccccc;"
        "}"
        "QMenu::item {"
        "    padding: 5px 20px 5px 20px;"
        "    color: #333333;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #e6f2ff;"
        "    color: #0066cc;"
        "}"
    );

    if (item && item->parent() && item->parent()->text() == "数据库连接") {
        // 右键点击的是连接项（根节点的子项）
        QString connName = item->text();

        contextMenu.addAction("连接");

        contextMenu.addAction("连接1");

        contextMenu.addAction("连接2");
    }

    // 添加新建连接选项
    contextMenu.addAction("新建连接", this, &MainWindow::createNewConnection);

    contextMenu.exec(ui->treeView->viewport()->mapToGlobal(pos));
}
// 加载所有连接
void MainWindow::loadAllConnections(QStandardItem *parentItem)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (const QString &connName, groups) {
        settings.beginGroup(connName);

        QStandardItem *connItem = new QStandardItem(QIcon(":/img/database.png"), connName);
        connItem->setData(connName, Qt::UserRole);

        // 添加连接信息作为工具提示
        QString tooltip = QString("主机: %1\n端口: %2\n用户: %3")
                            .arg(settings.value("host").toString())
                            .arg(settings.value("port").toString())
                            .arg(settings.value("user").toString());
        connItem->setToolTip(tooltip);

        parentItem->appendRow(connItem);
        settings.endGroup();
    }
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btn_connection) {
        if (event->type() == QEvent::Enter) {
            // 鼠标移入，放大
            QRect startRect = ui->btn_connection->geometry();
            QRect endRect = startRect.adjusted(-2, -2, 2, 2); // 放大 2px
            QPropertyAnimation *anim = new QPropertyAnimation(ui->btn_connection, "geometry");
            anim->setDuration(150);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(startRect);
            anim->setEndValue(endRect);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (event->type() == QEvent::Leave) {
            // 鼠标移出，恢复
            QRect endRect = ui->btn_connection->geometry();
            QRect startRect = endRect.adjusted(2, 2, -2, -2); // 恢复原大小
            QPropertyAnimation *anim = new QPropertyAnimation(ui->btn_connection, "geometry");
            anim->setDuration(150);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(endRect);
            anim->setEndValue(startRect);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::on_btn_create_clicked()
{
    cre->show();
}

void MainWindow::createNewConnection()
{
    cre->show();
}
