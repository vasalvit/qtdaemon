#! [module]
QT += daemon
#! [module]

#! [link]
LIBS += -L/qdaemon_instal_dir
LIBS += -lqdaemon

macx:QMAKE_RPATHDIR += /qdaemon_instal_dir/
#! [link]
