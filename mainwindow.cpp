#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitUi();
    test();
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
}
void MainWindow::test()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("123456");
    db.setDatabaseName("lhlist");
    if (!db.open()) {
        qDebug() << "lhdb is open sucessful!";
    } else {
        qDebug() << "lhdb is open failed!";
    }
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "Available database drivers:";
    foreach(const QString &driver, drivers) {
        qDebug() << " - " << driver;
    }
}
void MainWindow::ConnectSlot()
{

}

void MainWindow::on_btn_sql_mar_clicked()
{
    auto *m = new MySql(this);
    ui->stackedWidget->insertWidget(0, m);
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_btn_bom_clicked()
{

}

