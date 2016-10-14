#-------------------------------------------------
#
# Project created by QtCreator 2016-10-08T15:11:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = logo-recognition
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    addlogo.cpp \
    settings.cpp

HEADERS  += mainwindow.h \
    addlogo.h \
    settings.h

FORMS    += mainwindow.ui \
    addlogo.ui \
    settings.ui

RESOURCES += \
    resources.qrc

LIBS += `pkg-config --libs opencv`
