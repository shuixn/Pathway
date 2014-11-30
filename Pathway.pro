#-------------------------------------------------
#
# Project created by QtCreator 2014-11-24T11:50:38
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pathway
TEMPLATE = app


SOURCES += main.cpp\
        pathway.cpp \
    tcpserver.cpp \
    tcpclient.cpp \
    startchat.cpp \
    innerchat.cpp \
    qmenubutton.cpp

HEADERS  += pathway.h \
    tcpserver.h \
    tcpclient.h \
    startchat.h \
    innerchat.h \
    qmenubutton.h

FORMS    += pathway.ui \
    tcpserver.ui \
    tcpclient.ui \
    innerchat.ui
RESOURCES += application.qrc
