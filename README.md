# Quick summary #
A library facilitating the creation of daemons (linux) or services (windows).

# Description #

There are two exposed classes `QDaemonApplication` and `QDaemonLog`.

`QDaemonApplication` is derived from `QCoreApplication` and provides the basic infrastructure for the daemon.

`QDaemonApplication` exposes 5 signals:

* `daemonized(QStringList)` - emitted when the application is started as a daemon/service by the OS and notifies the user that initializations (like connecting signals/slots) can be done. The string list passed is a proper command line (can be used with `QCommandLineParser`) set for the daemon when installing.
* `started()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service has started.
* `stopped()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service has stopped.
* `installed()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service has been installed.
* `uninstalled()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service been uninstalled.

`QDaemonLog` is the logging component for the daemon. It's set up to output on `stdout` when the application is run as controlling terminal, and to a file (named after the application with .log extension) when the application is ran as daemon/service.
The logging component can be used by the user through `QDaemonLog & qDaemonLog();` coupled with `QDaemonLog & QDaemonLog::operator << (const QString &)` or `void qDaemonLog(const QString &, QDaemonLog::EntrySeverity)`. Currently the format of the output messages is fixed.
The `QDaemonLog` methods (and global functions) are **thread-safe**.

# Dependencies #

Linux: `QtCore` and `QtDBus` modules are required

Windows: `QtCore` + the native libraries `AdvApi` and `User32` (headers and import library files should be provided through the windows SDKs)

# Building #

Build as any one Qt compatible library.

Run `qmake` and then `make`/`nmake` or equivalent.

# Running #

The application behaviour is controlled through command line switches. When no switches are passed the application tries to run as a daemon. With command line switches the application works as a controlling terminal for the daemon/service.

* `--start`, `-s` Run the application as controlling terminal and attempt to start the daemon/service.

    Linux only (use the init.d script instead):

    * Additional command line arguments can be passed after adding `--`, signifying end of daemon arguments, however the generated init.d script should be preferred for controlling the daemon

    Windows only:

    * The additional command line arguments for the service can be specified when it is installed

* `--stop`, `-t` Run the application as controlling terminal and attempt to stop the daemon/service.

    Linux only:

    Use the init.d script instead of directly invoking the applicaton

* `--install`, `-i` Run the application as controlling terminal and attempt to install the daemon/service. Additional command line parameters for the daemon/service can be specified after `--`, which signifies the end of command line processing for the controlling application.

    Linux only:

    * `--dbus-prefix=<path>` provide a prefix path for the d-bus configuration file.
    * `--initdPrefix=<path>` provide a prefix path for the `init.d` script.

    Windows only:

    * `--update-path` whether the service should add its application directory to the windows PATH (it is appended if it doesn't exist)

* `--uninstall`, `-u` Run the application as controlling terminal and attempt to uninstall the daemon/service.

    Linux only (use the init.d script instead):

    * `--dbus-prefix=<path>` provide a prefix path for the d-bus configuration file.
    * `--initdPrefix=<path>` provide a prefix path for the `init.d` script.

    Windows only:

    * `--update-path` whether the service should remove its application directory from the windows PATH (**treat with care, as it may be a directory shared by multiple programs**).

* `--help`, `-h` Provide help text on the command line switches.
* `--fake` Runs in pseudo-daemon mode. The application object will emit the `daemonized(QStringList)` signal, but will not try to detach itself from the running terminal (Linux) and will not contact the service control manager (Windows). It is provided as a means to debug the daemon/service. Additional command line parameters for the daemon/service can be specified after `--`, which signifies the end of command line processing for the controlling application.

# Daemon/service installation #

Aside from using the specified installation/uninstallation switches there may be additional steps required to register the application.

Linux: after successfully running the application with `--install` it is still required to invoke `insserv` (or equivalent) command to register the generated `init.d` script.

# License #

The source code is distributed under the MIT license. See the `LICENSE` file for the full text.