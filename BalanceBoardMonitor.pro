#-------------------------------------------------
#
# Project created by QtCreator 2016-03-31T23:37:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = BalanceBoardMonitor
TEMPLATE = app

LIBS += -lxwiimote -lqcustomplot

SOURCES += main.cpp\
        bbmonitorwidget.cpp

HEADERS  += bbmonitorwidget.h

FORMS    += bbmonitorwidget.ui
