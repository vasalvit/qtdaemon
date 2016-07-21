INCLUDEPATH += ../../../include

LIBS += -L$$OUT_PWD/../../../lib

TEMPLATE = app

QT += core daemon

CONFIG += console
CONFIG -= app_bundle

target.path = $$[QT_INSTALL_EXAMPLES]/daemon/$$TARGET
INSTALLS += target
