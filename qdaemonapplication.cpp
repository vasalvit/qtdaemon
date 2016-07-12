#include "qdaemonapplication.h"
#include "private/qdaemonapplication_p.h"
#include "private/qabstractdaemonbackend.h"

#include <QScopedPointer>

QT_BEGIN_NAMESPACE

using namespace QtDaemon;
/*!
    \page QtDaemon library overview

    \section building Building the library

    To build the library for dynamic linking invoke \c qmake on the project file.
    \code
    $ qmake qdaemon.pro
    \endcode

    And then use the appropriate \c make command to build the generated
    \c qdaemon.make makefile.
    For example on Linux:
    \code
    $ make -f qdaemon.make
    \endcode

    \note To build as a static library provide the \c staticlib configuration
    to \c qmake.
    \code
    $ qmake qdaemon.pro CONFIG += staticlib
    \endcode

    \subsection dependencies Dependencies

    \bold{The library requires Qt 5.6 or later.}

    \table 100 %
        \header
            \li Platform
            \li Dependencies
        \row
            \li Linux
            \li \c{QtCore, QtDBus}
        \row
            \li Windows
            \li \c{QtCore, AdvApi, User32}
        \row
            \li OSX
            \li \c{QtCore}
    \endtable

    \note On Linux the library requires a running \c dbus daemon to operate
    correctly.

    \note On Windows the library requires two native libraries to be linked.
    Headers and import library files (\c{advapi.lib, user32.lib}) should
    be provided through the appropriate Windows SDK.

    \section installation Installation

    \section troubleshoot Commonly encountered problems

    \section license License
    The source code is distributed under the MIT license. See the LICENSE file
       for the full text.
*/

/*!
    \class QDaemonApplication

    \brief The \c QDaemonApplication class provides the basic infrastructure for
    developing system-level services (deamons on *nix).

    The \c QDaemonApplication class processes the application command line and
    provides both the daemon itself and the controlling application.

    The application behavior is controlled through command line switches
    passed on startup. When no switches are provided the command line help text
    is displayed. The application object will emit the appropriate signal
    associated with a switch when there are no errors processing the command line.

    The following command line switches are processed by the daemon application
    object:

    \table 100 %
        \header
            \li Switch
            \li Description
        \row
            \li \c {--start}
            \li {1, 2} Run the application as controlling terminal and attempt
                to start the daemon.
        \row
            \li \c {-s}
        \row
            \li \c {--stop}
            \li {1, 2} Run the application as controlling terminal and attempt
                to stop the daemon.
        \row
            \li \c {-t}
        \row
            \li \c {--install}
            \li {1, 2} Run the application as controlling terminal and attempt
                to install the daemon/service. Additional command line parameters
                for the daemon/service can be specified after \c --, which
                signifies the end of command line processing for the controlling
                application.
        \row
            \li \c {-i}
        \row
            \li \c {--uninstall}
            \li {1, 2} Run the application as controlling terminal and attempt
                to uninstall the daemon/service.
        \row
            \li \c {-u}
        \row
            \li \c {--help}
            \li {1, 2} Provide help text on the command line switches.
        \row
            \li \c {-h}
        \row
            \li \c {--fake}
            \li Runs in pseudo-daemon mode. The application object will emit
                the daemonized() signal, but will not try to detach itself
                from the terminal (Linux) and will not contact the service
                control manager (Windows). It is provided as a means to debug
                the daemon application. Additional command line parameters for
                the daemon can be specified after \c --, which signifies the
                end of command line processing for the controlling application.
    \endtable

    \sa QCoreApplication
*/
/*!
    \fn QDaemonApplication::daemonized(const QStringList & arguments)

    This signal is emitted when the process has been daemonized.
    The string list argument \a arguments is a proper command line that
    can be used with a QCommandLineParser instance.
*/
/*!
    \fn QDaemonApplication::started()

    This signal is emitted when the application is run as a control terminal
    and the daemon process is started successfully.
*/
/*!
    \fn QDaemonApplication::stopped()

    This signal is emitted when the application is run as a control terminal
    and the daemon process is stopped successfully.
*/
/*!
    \fn QDaemonApplication::installed()

    This signal is emitted when the application is run as a control terminal
    and the daemon application has been installed successfully.
*/
/*!
    \fn QDaemonApplication::uninstalled()

    This signal is emitted when the application is run as a control terminal
    and the daemon application has been uninstalled successfully.
*/

/*!
    Constructs the daemon application object.

    The \a argc and \a argv arguments are processed by the application and are
    made available through the \l{QDaemonApplication::}{daemonized()} signal.

    \warning The data referred to by \a argc and \a argv must stay valid
    for the entire lifetime of the application object.

    \warning Do not rely on the arguments() function as there are command-line
    parameters used internally by the class.
*/
QDaemonApplication::QDaemonApplication(int & argc, char ** argv)
    : QCoreApplication(argc, argv), d_ptr(new QDaemonApplicationPrivate(this))
{
}

/*!
    Destroys the QDaemonApplicaiton object.
*/
QDaemonApplication::~QDaemonApplication()
{
    delete d_ptr;
}

/*!
    Starts the application event loop and executes the daemon application.

    \sa quit(), exit(), processEvents(), QCoreApplication::exec()
*/
int QDaemonApplication::exec()
{
    // Just check if someone forgot to create the application object
    QDaemonApplication * app = QDaemonApplication::instance();
    if (Q_UNLIKELY(!app))  {
        qWarning("You must construct a QDaemonApplication before calling QDaemonApplication::exec.");
        return -1;
    }

    QStringList arguments = QDaemonApplication::arguments();
    QDaemonApplicationPrivate * const d = app->d_ptr;

    QCommandLineOption daemonOption(QStringLiteral("d"));
    daemonOption.setHidden(true);

    d->parser.addOption(daemonOption);
    d->parser.parse(arguments);

    // Create the appropriate backend
    bool isDaemon = d->parser.isSet(daemonOption);
    QScopedPointer<QAbstractDaemonBackend> backend(d->createBackend(isDaemon));

    // Reparse with the options that the backends may have added in their constructors
    d->parser.parse(arguments);

    return backend->exec();
}

/*!
    Returns a pointer to the application's QDaemonApplication instance.

    If no instance has been created, \c null is returned.
*/
QDaemonApplication * QDaemonApplication::instance()
{
    return qobject_cast<QDaemonApplication *>(QCoreApplication::instance());
}

/*!
    \property QDaemonApplication::autoQuit
    \brief Holds whether the daemon application should automatically exit the event loop.

    If the property is set to \c true and the application process is running as control terminal
    the application will quit automatically after emitting the proper signal or reporting errors.

    In case you need to attach some code that should be run after the daemon has started/stopped
    or after it has been installed/uninstalled you can disable the controlling process' auto
    quit feature and subscribe to the appropriate signal.

    By default this property is \c false.

    \note The property is enforced to \c false when the application is run with \c --fake.
    \note The property is enforced to \c true when the command line help is requested.

    \sa setAutoQuit()
*/

/*!
    Retrieves the value of the \c autoQuit property.

    \sa setAutoQuit()
*/
bool QDaemonApplication::autoQuit() const
{
    Q_D(const QDaemonApplication);
    return d->autoQuit;
}

/*!
    Sets the \c autoQuit property to \a enable.

    \sa autoQuit()
*/
void QDaemonApplication::setAutoQuit(bool enable)
{
    Q_D(QDaemonApplication);
    d->autoQuit = enable;
}

/*!
    Retrieves the application's description string.

    \sa setApplicationDescription()
*/
QString QDaemonApplication::applicationDescription()
{
    return QDaemonApplicationPrivate::description;
}

/*!
    Sets the description of the daemon application to \a description.

    \note The description is not required, but if provided it will be used with the service control manager (windows) or the init.d script (linux).

    \sa applicationDescription()
*/
void QDaemonApplication::setApplicationDescription(const QString & description)
{
    QDaemonApplicationPrivate::description = description;
}

QT_END_NAMESPACE
