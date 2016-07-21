!include(../examples.pri)  {
    error("Couldn't find the examples.pri file!")
}

QT += network

TARGET = tcpserver

SOURCES += main.cpp \
    tcpsession.cpp \
    tcpserver.cpp

HEADERS += \
    tcpsession.h \
    tcpserver.h
