# Quick summary #
A library facilitating the creation of daemons (linux) or services (windows).

# Description #

There are two exposed classes `QDaemonApplication` and `QDaemonLog`.

`QDaemonApplication` is derived from `QCoreApplication` and provides the basic infrastructure for the daemon.

`QDaemonApplication` exposes 5 signals:

* `daemonized()` - emitted when the application is started as a daemon/service by the OS and notifies the user that initializations (like connecting signals/slots) can be done.
* `started()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/servicehas started.
* `stopped()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service has stopped.
* `installed()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service has been installed.
* `uninstalled()` - emitted when the application is run as a controlling terminal, notifying the user that the daemon/service been uninstalled.

`QDaemonLog` is the logging component for the daemon. It's set up to output on `stdout` when the application is run as controlling terminal, and to a file (named after the application with .log extension) when the application is ran as daemon/service.
The logging component can be used by the user through `QDaemonLog & qDaemonLog();` coupled with `QDaemonLog & << (const QString &)` or `void qDaemonLog(const QString &, QDaemonLog::EntrySeverity)`. Currently the format of the output messages is fixed.

# Dependencies #

Linux: `QtCore` and `QtDBus` modules are required

Windows: `QtCore` + the native libraries `AdvApi` and `User32` (headers and import library files should be provided through the windows SDKs)

# Building #

Build as any one Qt compatible library.

Run `qmake` and then `make`/`nmake` or equivalent.

# Running #

The application behaviour is controlled through command line switches. When no switches are passed the application tries to run as a daemon. With command line switches the application works as a controlling terminal for the daemon/service.

* `--start`, `-s` Run the application as controlling terminal and attempt to start the daemon/service.
* `--stop`, `-t` Run the application as controlling terminal and attempt to stop the daemon/service.
* `--install`, `-i` Run the application as controlling terminal and attempt to install the daemon/service.

    Linux only:

    * `--dbus-prefix=<path>` provide a prefix path for the d-bus configuration file.
    * `--initdPrefix=<path>` provide a prefix path for the `init.d` script.

* `--uninstall`, `-u` Run the application as controlling terminal and attempt to uninstall the daemon/service.
* `--help`, `-h` Provide help text on the command line switches.
* `--fake` Runs in pseudo-daemon mode. The application object will emit the `daemonized()` signal, but will not try to detach itself from the running terminal (Linux) and will not contact the service control manager (Windows). It is provided as a means to debug the daemon/service.

# Daemon/service installation #

Aside from using the specified installation/uninstallation switches there may be additional steps required to register the application.

Linux: after successfully running the application with `--install` it is still required to invoke `insserv` (or equivalent) command to register the generated `init.d` script.

Windows: after removal of the service the PATH is currently left unchanged (meaning it has to be cleaned manually).

# License #

The source code is distributed under the MIT license. See `LICENSE.txt` for the full text.