#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QMainWindow>
#include <login.h>
#define DPI_ENABLE 1
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qssFile(":/sytle.qss");
    if (qssFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(qssFile.readAll());
    } else {
        qDebug() << "Failed to load QSS:" << qssFile.errorString();
    }

//    login dlg;                          // 1) 先弹登录
//    if (dlg.exec() != QDialog::Accepted)  // 2) 取消/关闭就直接退出
//        return 0;

    MainWindow w;                       // 3) 登录通过再创建主窗体
    w.show();
    return a.exec();                    // 4) 只启动一次事件循环
}

