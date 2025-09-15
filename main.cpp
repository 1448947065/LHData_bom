#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QMainWindow>
#define DPI_ENABLE 1
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QFile qssFile("D:\\work project\\sql\\untitled1\\sytle.qss");
    if (qssFile.open(QFile::ReadOnly)) {
        QString styleSheet = qssFile.readAll();
        a.setStyleSheet(styleSheet);
        qssFile.close();
    } else {
        qDebug() << "Failed to load QSS:" << qssFile.errorString(); // 打印错误
    }
    MainWindow w;
    w.resize(1000, 200);
    w.show();
    return a.exec();
}
