#-------------------------------------------------
#
# Project created by QtCreator 2014-11-24T11:50:38
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pathway
TEMPLATE = app


SOURCES += main.cpp\
        pathway.cpp \
    tcpserver.cpp \
    tcpclient.cpp

HEADERS  += pathway.h \
    tcpserver.h \
    tcpclient.h

FORMS    += pathway.ui \
    tcpserver.ui \
    tcpclient.ui
