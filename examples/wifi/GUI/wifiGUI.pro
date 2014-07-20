#-------------------------------------------------
#
# Project created by QtCreator 2014-05-10T15:01:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wifiGUI
TEMPLATE = app

SOURCES  += \
    main.cpp \
    mainwindow.cpp \
    visualizer.cpp \
    addnodewindow.cpp \
    aboutwindow.cpp \
    toolbar.cpp \
    centralwidget.cpp \
    experimentsetup.cpp \
    experimentmanager.cpp \
    schedulingvisualizer.cpp \
    statisticsvisualizer.cpp

HEADERS  += \
    addnodewindow.hpp \
    aboutwindow.hpp \
    toolbar.hpp \
    mainwindow.hpp \
    visualizer.hpp \
    centralwidget.hpp \
    experimentsetup.hpp \
    experimentmanager.hpp \
    schedulingvisualizer.hpp \
    statisticsvisualizer.hpp

FORMS    += mainwindow.ui \
    addnodewindow.ui \
    aboutwindow.ui \
    centralwidget.ui

OTHER_FILES +=

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += ../\
              ../../../src/

LIBS +=      -L$$_PRO_FILE_PWD_/../\
             -lwifi\
             -L$$_PRO_FILE_PWD_/../../../src/.libs/\
             -lmetasim
