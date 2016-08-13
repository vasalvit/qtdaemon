!include(../examples.pri)  {
    error("Couldn't find the examples.pri file!")
}

TARGET = tcpserver

QT += network

SOURCES += main.cpp \
    tcpsession.cpp \
    tcpserver.cpp

HEADERS += \
    tcpsession.h \
    tcpserver.h
