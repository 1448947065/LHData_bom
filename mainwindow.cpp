#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitUi();
    initiazeTreeView();
    setLabelStatus();

    //ui->lab_host->setText("NOT CONNECT");
}

MainWindow::~MainWindow()
{
    delete ui;
    Insert->close();
}

void MainWindow::InitUi()
{
    ui->btn_sql_mar->setText(tr("数据库"));
    ui->btn_bom->setText(tr("双清单管理工具"));
    ui->btn_hyper->setText(tr("超链接提取工具"));
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
//    setStyleSheet("background: transparent;");

    QIcon qicon;
    qicon.addFile(":/img/coc.png");
    ui->btn_connection->setIcon(qicon);
    ui->btn_connection->setIconSize(QSize(45,45));
    ui->btn_connection->setFixedSize(50, 50);

    QIcon qicon1;
    qicon1.addFile(":/img/sec.png");
    ui->btn_search->setIcon(qicon1);
    ui->btn_search->setIconSize(QSize(45,45));
    ui->btn_search->setFixedSize(50, 50);

    QIcon qicon2;
    qicon2.addFile(":/img/cre.png");
    ui->btn_create->setIcon(qicon2);
    ui->btn_create->setIconSize(QSize(40,40));
    ui->btn_create->setFixedSize(50, 50);

    QIcon qicon3;
    qicon3.addFile(":/img/del.png");
    ui->btn_del->setIcon(qicon3);
    ui->btn_del->setIconSize(QSize(45,45));
    ui->btn_del->setFixedSize(50, 50);

    QIcon qicon4;
    qicon4.addFile(":/img/cha.png");
    ui->btn_change->setIcon(qicon4);
    ui->btn_change->setIconSize(QSize(40,40));
    ui->btn_change->setFixedSize(50, 50);

    QIcon qicon5;
    qicon5.addFile(":/img/export.png");
    ui->btn_export->setIcon(qicon5);
    ui->btn_export->setIconSize(QSize(40,40));
    ui->btn_export->setFixedSize(50, 50);

    auto *root = new QWidget(this);
    root->setAttribute(Qt::WA_StyledBackground, true);
    root->setStyleSheet("background: transparent;");

    auto *rootLay = new QVBoxLayout(root);
    rootLay->setContentsMargins(16, 16, 16, 16);
    rootLay->setSpacing(0);

    card = new QWidget(root);
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground, true);
    card->setStyleSheet("#card{ background:white; border-radius:12px; }");

    effect  = new QGraphicsDropShadowEffect(card);
    effect->setBlurRadius(32);
    effect->setOffset(0, 6);
    effect->setColor(QColor(0, 0, 0, 70));
    card->setGraphicsEffect(effect);
    card->setObjectName("card");
    rootLay->addWidget(card);

    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(0, 0, 0, 0);
    cardLay->setSpacing(0);

    auto *title = new TitleBar(card);
    cardLay->addWidget(title);

    cardLay->addWidget(ui->centralwidget);

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

    connect(con, &Connection::sig_dbInfo, dbManager, &MySql::handleDbInfo);
    ui->btn_connection->installEventFilter(this);
    connect(con, &Connection::savedConnection,
            this, &MainWindow::onConnectionSaved);
    connect(this, &MainWindow::dbmes,
            Insert, &ExportInsert::getDbmes);
    ui->btn_export->setEnabled(false);
    ui->btn_change->setEnabled(false);
    ui->btn_search->setEnabled(false);
    ui->btn_del->setEnabled(false);
    ui->btn_create->setEnabled(false);
}
void MainWindow::onConnectionSaved(const QString &connName)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.beginGroup(connName);
    const QString host = settings.value("host").toString();
    const QString port = settings.value("port").toString();
    const QString user = settings.value("user").toString();
    settings.endGroup();

    QStandardItem *rootItem = treeModel->item(0);
    if (!rootItem) return;

    QStandardItem *connItem = findConnectionItem(connName);
    if (!connItem) {
        connItem = new QStandardItem(QIcon(":/img/database.png"), connName);
        connItem->setData(connName, Qt::UserRole + 1);
        connItem->setData("connection", Qt::UserRole + 2);
        rootItem->appendRow(connItem);

        for (int r = rootItem->rowCount() - 1; r >= 0; --r) {
            auto *it = rootItem->child(r);
            if (it && !it->isEnabled()) rootItem->removeRow(r);
        }
    }

    const QString tip = QString("主机: %1\n端口: %2\n用户: %3\n状态: 未连接")
                        .arg(host).arg(port).arg(user);
    connItem->setToolTip(tip);

    ui->treeView->expand(rootItem->index());
    ui->treeView->setCurrentIndex(connItem->index());
}

void MainWindow::insertDatabase()
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
//            qDebug() << "---------------------";
//        }
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->btn_connection) {
        if (event->type() == QEvent::Enter) {
            QSize startSize = ui->btn_connection->size();
            QSize endSize = startSize * 1.2;

            QPropertyAnimation *anim = new QPropertyAnimation(ui->btn_connection, "size");
            anim->setDuration(200);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(startSize);
            anim->setEndValue(endSize);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        } else if (event->type() == QEvent::Leave) {
            QSize endSize = ui->btn_connection->size();
            QSize startSize = endSize * 0.8333;

            QPropertyAnimation *anim = new QPropertyAnimation(ui->btn_connection, "size");
            anim->setDuration(200);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->setStartValue(endSize);
            anim->setEndValue(startSize);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
    return QWidget::eventFilter(obj, event);
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
    ui->stackedWidget->setCurrentIndex(1);
    QString programPath = "\"D:/work project/bomlist/build-bomlist-msvc2015-Debug/debug/bomlist.exe\"";
    QProcess::startDetached(programPath);

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

}
void MainWindow::initiazeTreeView()
{
    qDebug() << "======== 初始化树状视图 ========";

    if (treeModel) {
        delete treeModel;
    }
    treeModel = new QStandardItemModel(this);
    ui->treeView->setModel(treeModel);

    ui->treeView->show();
    ui->treeView->activateWindow();
    ui->treeView->raise();
    ui->treeView->setFocus();

    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionBehavior(QTreeView::SelectRows);
    ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
    ui->treeView->setIndentation(10);
    ui->treeView->setExpandsOnDoubleClick(false);
    ui->treeView->setItemsExpandable(true); // 保险起见
    QStandardItem *rootItem = new QStandardItem(QIcon(":/img/co.png"), "数据库连接");
    treeModel->appendRow(rootItem);

    loadSavedConnections(rootItem);

    bool conn1 = connect(ui->treeView, &QTreeView::doubleClicked,
                        this, &MainWindow::onConnectionDoubleClicked,
                        Qt::UniqueConnection);

    bool conn2 = connect(ui->treeView, &QTreeView::clicked,
                        [](const QModelIndex &idx){ qDebug() << "单击事件:" << idx; });

    qDebug() << "信号连接状态 - 双击:" << conn1 << "单击:" << conn2;

    ui->treeView->installEventFilter(this);
    qDebug() << "树状视图初始化完成 - 可见:" << ui->treeView->isVisible()
             << "激活:" << ui->treeView->isActiveWindow();
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::onTreeViewContextMenu);
    connect(ui->treeView, &QTreeView::expanded,  this, [this](const QModelIndex& idx){
        if (auto *it = treeModel->itemFromIndex(idx)) {
            const QString type = it->data(Qt::UserRole + 2).toString();
            if (type == "database") {
                QIcon ico(":/img/databaseopen.png");
                if (!ico.isNull()) it->setIcon(ico);
            } else if (type == "connection") {
                QIcon ico(":/img/cmysql.png");
                if (!ico.isNull()) it->setIcon(ico);
            }
        }
    });
    connect(ui->treeView, &QTreeView::collapsed, this, [this](const QModelIndex& idx){
        if (auto *it = treeModel->itemFromIndex(idx)) {
            const QString type = it->data(Qt::UserRole + 2).toString();
            if (type == "database") {
                QIcon ico(":/img/databaseclose.png");
                if (!ico.isNull()) it->setIcon(ico);
            } else if (type == "connection") {
                QIcon ico(":/img/mysql.png"); // 或你想要的“未展开/未连接”图标
                if (!ico.isNull()) it->setIcon(ico);
            }
        }
    });
}

void MainWindow::loadSavedConnections(QStandardItem *parentItem)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (const QString &connName, groups) {
        settings.beginGroup(connName);

        QStandardItem *connItem = new QStandardItem(QIcon(":/img/mysql.png"), connName);
        connItem->setData(connName, Qt::UserRole + 1);
        connItem->setData("connection", Qt::UserRole + 2);

        QString tooltip = QString("主机: %1\n端口: %2\n用户: %3\n状态: 未连接")
                            .arg(settings.value("host").toString())
                            .arg(settings.value("port").toString())
                            .arg(settings.value("user").toString());
        connItem->setToolTip(tooltip);

        parentItem->appendRow(connItem);
        settings.endGroup();
    }

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
    if (!item) return;

    QString itemType = item->data(Qt::UserRole + 2).toString();

    if (itemType == "connection") {

        QString connName = item->text();
        qDebug() << "双击连接项:" << connName;

        if (item->rowCount() > 0) {
            if (ui->treeView->isExpanded(index)) {
                ui->treeView->collapse(index);
            } else {
                ui->treeView->expand(index);
            }
        } else {
            connectToDatabase(connName);
            db = QSqlDatabase::database(connName);
            if (db.isOpen()) {
                // 延迟执行确保UI更新完成
                QTimer::singleShot(100, this, [this, connName](){
                });
            }
        }
    }
    if (!item->parent() && item->text() == "数据库连接") {
        ui->treeView->setExpanded(index, !ui->treeView->isExpanded(index));
        return;
    }
    else if (itemType == "database" || itemType == "category") {
        ui->btn_export->setEnabled(true);
        //ui->btn_export->setStyleSheet("");  // 恢复默认样式
        if (ui->treeView->isExpanded(index)) {
            ui->treeView->collapse(index);
            // 数据库连接成功后恢复按钮可用状态
            if (itemType == "database") {
                item->setIcon(QIcon(":/img/databaseclose.png")); // 收起时
                ui->btn_export->setEnabled(false);
            }
        } else {
            ui->treeView->expand(index);
            if (itemType == "database") {
                item->setIcon(QIcon(":/img/databaseopen.png"));  // 展开时
            }
        }
    }
    else if (itemType == "table") {
        QStandardItem *tableItem = item;
        QStandardItem *tablesItem = tableItem->parent();
        QStandardItem *dbItem = tablesItem ? tablesItem->parent() : nullptr;
        QStandardItem *connItem = dbItem ? dbItem->parent() : nullptr;

        if (!dbItem || !connItem) return;

        QString connName = connItem->text();
        QString dbName = dbItem->text();
        QString tableName = tableItem->text();

        QSqlDatabase db = QSqlDatabase::database(connName);
        if (!db.isOpen()) {
            QMessageBox::warning(this, "错误", "数据库连接已断开");
            return;
        }

        db.setDatabaseName(dbName);
        if (!db.open()) {
            QMessageBox::critical(this, "错误", "无法打开数据库: " + db.lastError().text());
            return;
        }

        if (ui->tableView->model()) {
            delete ui->tableView->model();
        }

        QSqlTableModel *model = new QSqlTableModel(this, db);
        model->setTable(tableName);
        model->setEditStrategy(QSqlTableModel::OnRowChange);

        if (!model->select()) {
            QMessageBox::critical(this, "错误", "无法加载表数据: " + model->lastError().text());
            delete model;
            return;
        }

        ui->tableView->setModel(model);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->setStyleSheet(
            "QTableView {"
            "    background-color: #ffffff;"
            "    alternate-background-color: #f5f9ff;"  /* 浅蓝色交替行 */
            "    gridline-color: #e0e0e0;"
            "    border: none;"
            "    font-size: 12px;"
            "}"
            "QTableView::item {"
            "    padding: 6px;"
            "    border-bottom: 1px solid #f0f0f0;"  /* 项之间的分隔线 */
            "}"
            "QTableView::item:selected {"
            "    background-color: #e6f2ff;"
            "    color: #0066cc;"
            "}"
            "QHeaderView::section {"
            "    background-color: #f8f8f8;"
            "    padding: 6px;"
            "    border: 1px solid #e0e0e0;"
            "}"
        );
        if (!m_proxy) {
            m_proxy = new QSortFilterProxyModel(this);
            m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
            m_proxy->setFilterKeyColumn(-1);
            ui->tableView->setModel(m_proxy);
            ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
            ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            ui->tableView->setStyleSheet(
                "QTableView::item:selected { background:#fff3b0; color:black; }"
            );
        }
        m_proxy->setSourceModel(model);
        ui->tableView->setAlternatingRowColors(true);
        ui->tableView->resizeColumnsToContents();

        qDebug() << "显示表数据:" << dbName << "." << tableName;
    }
}

void MainWindow::connectToDatabase(const QString &connName)
{

    QSettings settings("config.ini", QSettings::IniFormat);
    const QStringList groups = settings.childGroups();
    if (!groups.contains(connName)) {
        QMessageBox::warning(this, "配置缺失",
                             QString("未在 config.ini 找到分组 [%1]").arg(connName));
        return;
    }

    settings.beginGroup(connName);
    QString host = settings.value("host").toString();
    quint16 port = settings.value("port").toUInt();
    QString user = settings.value("user").toString();
    QString pwd  = settings.value("password").toString();
    settings.endGroup();

    if (host.compare("localhost", Qt::CaseInsensitive) == 0)
        host = "127.0.0.1";

    qDebug() << "[TryConnect]" << connName << host << port << user;

    if (QSqlDatabase::contains(connName)) {
        {
            QSqlDatabase old = QSqlDatabase::database(connName, /* open */ false);
            if (old.isOpen()) old.close();
        }
        QSqlDatabase::removeDatabase(connName);
    }

    if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        QMessageBox::critical(this, "驱动缺失",
                              "未找到 QMYSQL 驱动（请检查 sqldrivers 目录和 MySQL 客户端库）。");
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(pwd);
    db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=2;MYSQL_OPT_RECONNECT=1");

    if (!db.open()) {
        QMessageBox::critical(this, "连接失败",
                              QString("无法连接到数据库：\n%1").arg(db.lastError().text()));
        qDebug() << "[MySQL Open Fail]" << db.lastError().text();
        return;
    }

    if (QStandardItem *connItem = findConnectionItem(connName)) {
        QString tip = connItem->toolTip();
        if (tip.isEmpty()) {
            tip = QString("主机: %1\n端口: %2\n用户: %3\n状态: 已连接 ✓").arg(host).arg(port).arg(user);

            //ui->lab_host->setText(host);
        }


        else
            tip.replace("状态: 未连接", "状态: 已连接 ✓");
        connItem->setToolTip(tip);

        if (connItem->rowCount() == 0) {
            QSqlQuery q(db);
            if (q.exec("SHOW DATABASES")) {
                while (q.next()) {
                    const QString dbName = q.value(0).toString();

                    if (QStandardItem *connItem = findConnectionItem(connName)) {
                        connItem->setIcon(QIcon(":/img/cmysql.png"));
                    }

                    auto *dbItem = new QStandardItem(QIcon(":/img/databaseclose.png"), dbName);
                    dbItem->setData(dbName, Qt::UserRole + 1);
                    dbItem->setData("database", Qt::UserRole + 2);

                    auto *tablesItem = new QStandardItem(QIcon(":/img/table.png"), "Tables");
                    tablesItem->setData("category", Qt::UserRole + 2);
                    tablesItem->setSelectable(false);

                    dbItem->appendRow(tablesItem);
                    connItem->appendRow(dbItem);

                    QSqlQuery tq(db);
                    if (tq.exec(QString("SHOW TABLES FROM `%1`").arg(dbName))) {
                        while (tq.next()) {
                            const QString tableName = tq.value(0).toString();
                            auto *tableItem = new QStandardItem(QIcon(":/img/table.png"), tableName);
                            tableItem->setData(tableName, Qt::UserRole + 1);
                            tableItem->setData("table", Qt::UserRole + 2);
                            tablesItem->appendRow(tableItem);
                        }
                    }
                }
            }
        }


        ui->lab_host->setText(host);
        ui->lab_port->setText(QString::number(port, 10));
        ui->lab_user->setText(user);
        ui->treeView->expand(connItem->index());
    }

    qDebug() << "[Connected]" << connName << "OK";
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

void MainWindow::onTreeViewContextMenu(const QPoint &pos)
{
    // 注意：pos 来自 viewport()
    QModelIndex index = ui->treeView->indexAt(pos);
    QStandardItem *item = treeModel->itemFromIndex(index);
    if (!item) return;

    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu { background-color: white; border: 1px solid #cccccc; }"
        "QMenu::item { padding: 5px 20px; color: #333333; }"
        "QMenu::item:selected { background-color: #e6f2ff; color: #0066cc; }"
    );

    const QString type = item->data(Qt::UserRole + 2).toString();

    // 根节点：数据库连接
    if (!item->parent() && item->text() == tr("数据库连接")) {
        contextMenu.addAction(tr("新建连接"), this, &MainWindow::createNewConnection);
        contextMenu.addAction(tr("刷新"), [this]{ initiazeTreeView(); });
    }
    // 连接节点
    else if (type == "connection") {
        const QString connName = item->text();

        contextMenu.addAction(tr("新建数据库"), [this, connName] {
            bool ok = false;
            const QString dbName =
                QInputDialog::getText(this, tr("新建数据库"), tr("请输入数据库名:"),
                                      QLineEdit::Normal, "", &ok);
            if (ok && !dbName.isEmpty()) {
                QSqlDatabase db = QSqlDatabase::database(connName);
                if (db.isOpen()) {
                    QSqlQuery query(db);
                    if (query.exec(QString("CREATE DATABASE `%1`").arg(dbName))) {
                        QMessageBox::information(this, tr("成功"), tr("数据库已创建！"));
                        connectToDatabase(connName);
                    } else {
                        QMessageBox::critical(this, tr("错误"),
                                              tr("创建失败: ") + query.lastError().text());
                    }
                }
            }
        });

        contextMenu.addAction(tr("删除连接"), [this, connName] {
            if (QMessageBox::question(this, tr("确认删除"),
                                      tr("确定删除连接 [%1] 吗？\n这会同时从 config.ini 中移除。")
                                      .arg(connName)) == QMessageBox::Yes) {
                deleteConnection(connName);
            }
        });
    }
    // 数据库节点
    else if (type == "database") {
        const QString dbName = item->text();
        QStandardItem *connItem = item->parent();
        if (connItem) {
            const QString connName = connItem->text();
            contextMenu.addAction(tr("删除数据库"), [this, connName, dbName] {
                QSqlDatabase db = QSqlDatabase::database(connName);
                if (db.isOpen()) {
                    QSqlQuery query(db);
                    if (query.exec(QString("DROP DATABASE `%1`").arg(dbName))) {
                        QMessageBox::information(this, tr("成功"), tr("数据库已删除！"));
                        // 可选：initiazeTreeView();
                    } else {
                        QMessageBox::critical(this, tr("错误"),
                                              tr("删除失败: ") + query.lastError().text());
                    }
                }
            });
        }
    }
    // Tables 分类节点（之前被注释导致菜单空）
    else if (type == "category") {
        // 找到父数据库/连接
        QStandardItem *dbItem   = item->parent();
        QStandardItem *connItem = dbItem ? dbItem->parent() : nullptr;
        if (dbItem && connItem) {
            const QString dbName   = dbItem->text();
            const QString connName = connItem->text();

            contextMenu.addAction(tr("导入数据表..."), [this, connName, dbName] {
                if (!Insert) return;
                qDebug()<<"[mes]:connName:"<<connName <<" "<< dbName;
                emit dbmes(connName, dbName);
                Insert->setWindowTitle(QString(tr("导入数据表 → 数据库 %1")).arg(dbName));
                Insert->show(); Insert->raise(); Insert->activateWindow();
            });
        }
    }
    // 表节点
    else if (type == "table") {
        const QString tableName = item->text();
        // tables 的父节点是 database
        QStandardItem *dbItem   = item->parent() ? item->parent()->parent() : nullptr;
        QStandardItem *connItem = dbItem ? dbItem->parent() : nullptr;
        if (dbItem && connItem) {
            const QString dbName   = dbItem->text();
            const QString connName = connItem->text();

            // 导入数据到该表...
            contextMenu.addAction(tr("导入数据到该表..."),
                                  [this, connName, dbName, tableName] {
                if (!Insert) return;
                // 若 Insert 支持设定目标表可放开下面一行：
                // Insert->setTarget(connName, dbName, tableName);
                Insert->setWindowTitle(
                    QString(tr("导入数据 → %1.%2")).arg(dbName, tableName));
                Insert->show(); Insert->raise(); Insert->activateWindow();
            });

            // 删除表
            contextMenu.addAction(tr("删除表"),
                                  [this, connName, dbName, tableName, item] {
                QSqlDatabase db = QSqlDatabase::database(connName);
                if (db.isOpen()) {
                    if (db.databaseName() != dbName) { db.setDatabaseName(dbName); db.open(); }
                    QSqlQuery query(db);
                    if (query.exec(QString("DROP TABLE `%1`").arg(tableName))) {
                        QMessageBox::information(this, tr("成功"), tr("表已删除！"));
                        if (item->parent()) item->parent()->removeRow(item->row());
                    } else {
                        QMessageBox::critical(this, tr("错误"),
                                              tr("删除失败: ") + query.lastError().text());
                    }
                }
            });
        }
    }
    if (!contextMenu.actions().isEmpty()) {
        contextMenu.exec(ui->treeView->viewport()->mapToGlobal(pos));
    }
}


bool MainWindow::deleteConnection(const QString &connName)
{
    if (QSqlDatabase::contains(connName)) {
        {
            QSqlDatabase db = QSqlDatabase::database(connName, /* open */ false);
            if (db.isOpen()) db.close();
        }
        QSqlDatabase::removeDatabase(connName);
    }

    QSettings settings("config.ini", QSettings::IniFormat);
    settings.remove(connName);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        QMessageBox::warning(this, "提示", "已从界面移除，但写入配置文件失败。");
    }

    QList<QStandardItem*> items = treeModel->findItems(connName, Qt::MatchRecursive);
    for (QStandardItem *item : items) {
        if (item->data(Qt::UserRole + 2).toString() == "connection") {
            QStandardItem *parent = item->parent();
            if (parent) parent->removeRow(item->row());
            else        treeModel->removeRow(item->row());
            break;
        }
    }

    // 4) 若已无连接，补一个“暂无连接配置”的占位
    if (QStandardItem *root = treeModel->item(0)) {
        if (root->rowCount() == 0) {
            auto *emptyItem = new QStandardItem("暂无连接配置");
            emptyItem->setEnabled(false);
            emptyItem->setSelectable(false);
            root->appendRow(emptyItem);
        }
    }

    QMessageBox::information(this, "成功", QString("连接 %1 已删除（含配置）。").arg(connName));
    return true;
}

void MainWindow::loadAllConnections(QStandardItem *parentItem)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    foreach (const QString &connName, groups) {
        settings.beginGroup(connName);

        QStandardItem *connItem = new QStandardItem(QIcon(":/img/database.png"), connName);
        connItem->setData(connName, Qt::UserRole);

        QString tooltip = QString("主机: %1\n端口: %2\n用户: %3")
                            .arg(settings.value("host").toString())
                            .arg(settings.value("port").toString())
                            .arg(settings.value("user").toString());
        connItem->setToolTip(tooltip);

        parentItem->appendRow(connItem);
        settings.endGroup();
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized()) {
            card->setStyleSheet("#card { background:white; border-radius:0px; }");
            effect->setEnabled(false);

            if (auto rootLay = qobject_cast<QVBoxLayout*>(centralWidget()->layout())) {
                rootLay->setContentsMargins(0, 0, 0, 0);
            }
        } else {
            card->setStyleSheet("#card { background:white; border-radius:12px; }");
            effect->setEnabled(true);

            if (auto rootLay = qobject_cast<QVBoxLayout*>(centralWidget()->layout())) {
                rootLay->setContentsMargins(16, 16, 16, 16);
            }
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::on_btn_create_clicked()
{
    cre->show();
}

void MainWindow::createNewConnection()
{
    con->show();
}

void MainWindow::setTableStyle()
{
    QSqlTableModel *model = dbManager->getModel("lhbom");

    // 用代理包装一下
    auto *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1); // 全列匹配

    ui->tableView->setModel(proxy);

    // 设置表格样式（你原来的 QSS 保留）
    ui->tableView->setStyleSheet(
        "QTableView { background:white; }"
        "QTableView::item:selected { background:#fff3b0; color:black; }"
    );
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // 连接搜索框
    connect(ui->line_search, &QLineEdit::textChanged, this, [=](const QString &s){
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        proxy->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(s),
                                         QRegularExpression::CaseInsensitiveOption));
#else
        proxy->setFilterRegExp(QRegExp(QRegExp::escape(s), Qt::CaseInsensitive));
#endif

        // 高亮匹配
        auto *sel = ui->tableView->selectionModel();
        sel->clearSelection();
        if (s.isEmpty()) return;

        for (int r = 0; r < proxy->rowCount(); ++r) {
            for (int c = 0; c < proxy->columnCount(); ++c) {
                QModelIndex idx = proxy->index(r, c);
                QString text = proxy->data(idx).toString();
                if (text.contains(s, Qt::CaseInsensitive)) {
                    sel->select(idx, QItemSelectionModel::Select);
                }
            }
        }
        ui->tableView->viewport()->update();
    });
}

void MainWindow::setLabelStatus()
{

}

void MainWindow::on_btn_del_clicked()
{
    delui->show();
}

void MainWindow::on_btn_export_clicked()
{
    Insert->show();
}
void MainWindow::on_btn_serach_tab_clicked()
{
    QAbstractItemModel* model = ui->tableView->model();
    if (!model) return;

    const QString term = ui->line_search->text().trimmed();
    if (term.isEmpty()) { return; }

    if (term.compare(m_findTerm, Qt::CaseInsensitive) != 0) {
        m_findTerm = term;
        m_findRow = -1;
        m_findCol = -1;
    }

    const int rows = model->rowCount();
    const int cols = model->columnCount();
    if (rows == 0 || cols == 0) return;

    // 从下一个单元格开始找（行主序遍历）
    int r = m_findRow, c = m_findCol;
    // 计算起点：第一次点击从 (0,0)，否则从下一个
    if (r < 0 || c < 0) { r = 0; c = 0; }
    else {
        ++c;
        if (c >= cols) { c = 0; ++r; }
        if (r >= rows) { r = 0; c = 0; } // wrap 到开头
    }

    bool found = false;
    for (int step = 0; step < rows * cols; ++step) {
        QModelIndex idx = model->index(r, c);
        const QString text = model->data(idx, Qt::DisplayRole).toString();
        if (text.contains(term, Qt::CaseInsensitive)) {
            // 选中并滚动到视图中间
            auto *sel = ui->tableView->selectionModel();
            if (sel) {
                sel->clearSelection();
                sel->select(idx, QItemSelectionModel::ClearAndSelect);
                ui->tableView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                ui->tableView->setCurrentIndex(idx);
            }
            // 记录当前位置，下一次从这里的下一个开始
            m_findRow = r;
            m_findCol = c;
            found = true;
            break;
        }
        // 下一个单元格
        ++c;
        if (c >= cols) { c = 0; ++r; }
        if (r >= rows) { r = 0; }
    }

    if (!found) {
        // 没找到：清空状态并可选提示；再次点击会从头找
        m_findRow = -1;
        m_findCol = -1;
        // 可选：QMessageBox::information(this, "查找", "没有更多匹配，已回到起点。");
    }
}

void MainWindow::on_btn_hyper_clicked()
{
    QString programPath = "\"D:/work project/bomlist/build-bomlist-msvc2015-Debug/debug/超链接提取工具v1.0.0.exe\"";
    QProcess::startDetached(programPath);
}

