#-------------------------------------------------
#
# Project created by QtCreator 2014-07-10T09:28:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SchedulingVisualizer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    schedulingvisualizer.cpp

HEADERS  += mainwindow.hpp \
    schedulingvisualizer.hpp

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++0x
