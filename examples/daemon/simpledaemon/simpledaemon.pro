!include(../examples.pri)  {
    error("Couldn't find the examples.pri file!")
}

TARGET = simpledaemon

SOURCES += main.cpp \
    simpledaemon.cpp

HEADERS += \
    simpledaemon.h
