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
	private/qdaemonlog_p.cpp \
    private/controllerbackend_linux.cpp \
    private/qdaemonapplication_p.cpp \
    private/daemonbackend_linux.cpp \
    private/qabstractdaemonbackend.cpp

HEADERS += qdaemon-global.h \
	QDaemonApplication \
	QDaemonLog \
	qdaemonapplication.h \
	qdaemonlog.h \
	private/qdaemonapplication_p.h \
	private/qdaemonlog_p.h \
    private/daemonbackend_linux.h \
    private/controllerbackend_linux.h \
    private/qabstractdaemonbackend.h

unix  {
	SOURCES +=

	HEADERS +=

	target.path = /usr/lib
	INSTALLS += target

	DISTFILES += \
		resources/init \
		resources/dbus

	RESOURCES += \
		qdaemon.qrc
}
win32  {
	SOURCES += private/qdaemonbackend_win.cpp \
		private/qcontrollerbackend_win.cpp
	HEADERS += private/win/qdaemonbackend_win.h \
		private/qcontrollerbackend_win.h

	LIBS += -luser32 -ladvapi32
#	DESTDIR = $$OUT_PWD	# For shadow building on Windows
}


