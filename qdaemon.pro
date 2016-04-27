#-------------------------------------------------
#
# Project created by QtCreator 2016-04-21T15:42:46
#
#-------------------------------------------------

QT += core
QT -= gui

unix:!macx  {
	QT += dbus
}

macx|win32  {
	error("This library is currently not supporting your platform.")
}

TARGET = qdaemon
TEMPLATE = lib

DEFINES += QDAEMON_LIBRARY

MAKEFILE = qdaemon.make

SOURCES += qdaemonapplication.cpp \
	private/qdaemonapplication_p.cpp \
	private/qdaemonbackend_win.cpp \
	private/qdaemonbackend_unix.cpp \
	private/qdaemonbackend.cpp

HEADERS += qdaemonapplication.h \
	qdaemon-global.h \
	QDaemonApplication \
	private/qdaemonapplication_p.h \
	private/qdaemonbackend_win.h \
	private/qdaemonbackend_unix.h \
	private/qdaemonbackend.h

unix {
	target.path = /usr/lib
	INSTALLS += target
}

DISTFILES +=

RESOURCES += \
	qdaemon.qrc
