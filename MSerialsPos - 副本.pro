#-------------------------------------------------
#
# Project created by QtCreator 2019-05-23T22:39:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MSerialsPos
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    mediator.cpp \
    serials.cpp \
    dialogsetting.cpp \
    mcamera.cpp

HEADERS += \
        mainwindow.h \
    mediator.h \
    preference.h \
    serials.h \
    vardefine.h \
    global.h \
    dialogsetting.h \
    excv.h \
    mcamera.h \
    imagedeal.h \
    MSerialsCV.h

FORMS += \
        mainwindow.ui \
    dialogsetting.ui



INCLUDEPATH +=  ../third_party/opencv4.0.1/include \
                ../third_party/opencv4.0.1/include/opencv2 \
                ../third_party/halcon17/include \
                ../third_party/halcon17/include/halconcpp

INCLUDEPATH += /include/


QT += serialport

CONFIG += console
