#-------------------------------------------------
#
# Project created by QtCreator 2017-03-07T19:04:08
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tcpserver
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

CONFIG += link_pkgconfig

PKGCONFIG += opencv

LIBS += -L/opt/vc/lib/ -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -lRTIMULib

FORMS    += mainwindow.ui
