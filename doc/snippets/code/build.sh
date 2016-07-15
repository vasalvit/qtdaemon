//! [qmake]
qmake qdaemon.pro
//! [qmake]
//! [make]
make -f qdaemon.make
//! [make]
//! [qmake_static]
qmake qdaemon.pro CONFIG += staticlib
//! [qmake_static]


