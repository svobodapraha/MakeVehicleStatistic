#-------------------------------------------------
#
# Project created by QtCreator 2015-11-09T10:16:32
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MakeVehicleStatistic
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    lcstime.cpp \
    commonfunc.cpp \
    vehicle.cpp

HEADERS  += mainwindow.h \
    stdtypes.h \
    const.h \
    lcstime.h \
    commonfunc.h \
    vehicle.h

FORMS    += mainwindow.ui

# The application version
VERSION = 1.0.0.0

# Define the preprocessor macro to get the application version in our application.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"


unix|win32: LIBS += -L$$PWD/../SMTP_lib/ -lSMTPEmail

INCLUDEPATH += $$PWD/../SMTP_lib
DEPENDPATH += $$PWD/../SMTP_lib
