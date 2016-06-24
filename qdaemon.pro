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

!CONFIG(staticlib) {
  DEFINES += QDAEMON_LIBRARY
} else {
  DEFINES += QDAEMON_STATIC
}

MAKEFILE = qdaemon.make

SOURCES += qdaemonapplication.cpp \
	qdaemonlog.cpp \
	private/qdaemonlog_p.cpp \
	private/qdaemonapplication_p.cpp \
	private/qabstractdaemonbackend.cpp

HEADERS += qdaemon-global.h \
	QDaemonApplication \
	QDaemonLog \
	qdaemonapplication.h \
	qdaemonlog.h \
	private/qdaemonapplication_p.h \
	private/qdaemonlog_p.h \
	private/qabstractdaemonbackend.h

unix  {
	SOURCES += private/controllerbackend_linux.cpp \
		private/daemonbackend_linux.cpp

	HEADERS += private/controllerbackend_linux.h \
		private/daemonbackend_linux.h


	target.path = /usr/lib
	INSTALLS += target

	DISTFILES += \
		resources/init \
		resources/dbus

	RESOURCES += \
		qdaemon.qrc
}
win32  {
	SOURCES += private/controllerbackend_win.cpp \
		private/daemonbackend_win.cpp

	HEADERS += private/controllerbackend_win.h \
		private/daemonbackend_win.h

	LIBS += -luser32 -ladvapi32
#	DESTDIR = $$OUT_PWD	# For shadow building on Windows
}


