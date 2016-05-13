#-------------------------------------------------
#
# Project created by QtCreator 2016-04-21T15:42:46
#
#-------------------------------------------------

macx  {
	error("This library is currently not supporting your platform.")
}

QT += core
QT -= gui

unix:!macx  {
	QT += dbus
}

TARGET = qdaemon
TEMPLATE = lib

CONFIG += c++11

DEFINES += QDAEMON_LIBRARY

MAKEFILE = qdaemon.make

SOURCES += qdaemonapplication.cpp \
	qdaemonlog.cpp \
	private/qdaemonapplication_p.cpp \
	private/qdaemonbackend.cpp \
	private/qdaemonlog_p.cpp

HEADERS += qdaemon-global.h \
	QDaemonApplication \
	QDaemonLog \
	qdaemonapplication.h \
	qdaemonlog.h \
	private/qdaemonapplication_p.h \
	private/qdaemonbackend.h \
	private/qdaemonlog_p.h

unix  {
	SOURCES += private/qdaemonbackend_unix.cpp
	HEADERS += private/qdaemonbackend_unix.h

	target.path = /usr/lib
	INSTALLS += target

	DISTFILES += \
		resources/init \
		resources/dbus

	RESOURCES += \
		qdaemon.qrc
}
win32  {
	SOURCES += private/qdaemonbackend_win.cpp
	HEADERS += private/qdaemonbackend_win.h

	LIBS += -ladvapi32
#	DESTDIR = $$OUT_PWD	# For shadow building on Windows
}


