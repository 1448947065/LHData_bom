QT       += sql core gui xlsx

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    btnstyle.cpp \
    connection.cpp \
    createdatabase.cpp \
    deldatabase.cpp \
    dialog.cpp \
    exportinsert.cpp \
    lhxlsx.cpp \
    main.cpp \
    mainwindow.cpp \
    mysql.cpp \
    titlebar.cpp

HEADERS += \
    btnstyle.h \
    connection.h \
    createdatabase.h \
    deldatabase.h \
    dialog.h \
    exportinsert.h \
    lhxlsx.h \
    mainwindow.h \
    mysql.h \
    titlebar.h

FORMS += \
    connection.ui \
    createdatabase.ui \
    deldatabase.ui \
    dialog.ui \
    exportinsert.ui \
    mainwindow.ui

# QMAKE_CFLAGS  += /utf-8
QMAKE_CXXFLAGS += /utf-8
QXLSX_INCLUDE = $$[QT_INSTALL_HEADERS]/QtXlsx
QXLSX_LIB = $$[QT_INSTALL_LIBS]/Qt5Xlsx.lib
INCLUDEPATH += $${QXLSX_INCLUDE}
LIBS += $${QXLSX_LIB}
INCLUDEPATH += "C:/Program Files/MySQL/MySQL Server 8.0/include"
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    img/co.png \
    img/cre.png
