#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QMainWindow>
#include <login.h>
#define DPI_ENABLE 1
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QFile qssFile(":/sytle.qss");
    if (qssFile.open(QFile::ReadOnly)) {
        QString styleSheet = qssFile.readAll();
        a.setStyleSheet(styleSheet);
        qssFile.close();
    } else {
        qDebug() << "Failed to load QSS:" << qssFile.errorString(); // 打印错误
    }
    login w;
    if (w.exec() == QDialog::Accepted) {
        MainWindow mainUi;
        mainUi.show();
        return a.exec();
    }
    else {
        w.show();
        return a.exec();
    }
}
