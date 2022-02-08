QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bnf.cpp \
    codegenerator.cpp \
    dot.cpp \
    main.cpp \
    mainwindow.cpp \
    scan.cpp \
    parse.cpp \
    symtab.cpp \
    textoutputdialog.cpp

HEADERS += \
    bnf.h \
    codegenerator.h \
    dot.h \
    mainwindow.h \
    scan.h \
    parse.h \
    symtab.h \
    textoutputdialog.h

FORMS += \
    mainwindow.ui \
    textoutputdialog.ui

TRANSLATIONS += \
    MyMiniCC_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
