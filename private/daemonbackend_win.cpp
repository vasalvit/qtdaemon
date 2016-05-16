#include "daemonbackend_win.h"
#include "qdaemonlog.h"
#include "qdaemonapplication.h"

#include <QCommandLineParser>
#include <QThread>
#include <QSemaphore>

#include <Windows.h>

QT_BEGIN_NAMESPACE

static const qint32 serviceWaitHint = 1000;			// Just a service control sugar (1 second)

// --------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- WinAPI control dispatcher wrapper thread ------------------------------------------------------------------------------------------------------ //
// --------------------------------------------------------------------------------------------------------------------------------------------------- //

VOID WINAPI ServiceMain(DWORD, LPTSTR *);
DWORD WINAPI ServiceControlHandler(DWORD, DWORD, LPVOID, LPVOID);
VOID WINAPI ReportServiceStatus(DWORD, DWORD, DWORD);

class Q_DAEMON_LOCAL WindowsCtrlDispatcher : public QThread
{
	friend VOID WINAPI ServiceMain(DWORD, LPTSTR *);
	friend DWORD WINAPI ServiceControlHandler(DWORD, DWORD, LPVOID, LPVOID);
	friend VOID WINAPI ReportServiceStatus(DWORD, DWORD, DWORD);

public:
	WindowsCtrlDispatcher(const QString & service)
		: QThread(NULL), serviceName(service), serviceStartLock(0), serviceQuitLock(0), serviceStatusHandle(NULL)
	{
		Q_ASSERT(!instance);
		instance = this;
		::memset(dispatchTable, 0, 2 * sizeof(SERVICE_TABLE_ENTRY));
	}

	void run() override
	{
		dispatchTable[0].lpServiceName = reinterpret_cast<LPTSTR>(const_cast<ushort *>(serviceName.utf16()));	// Type safety be damned ...!
		dispatchTable[0].lpServiceProc = ServiceMain;

		// Just call the control dispatcher and hang
		if (!StartServiceCtrlDispatcher(dispatchTable))  {
			qDaemonLog(QStringLiteral("Couldn't run the service control dispatcher (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
			serviceStartLock.release();		// Free the main thread
		}

		QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);	// We want to quit (if we are not already quitting)
	}

	void WindowsCtrlDispatcher::startService()
	{
		start();						// Start the control thread
		serviceStartLock.acquire();		// Wait until the service has started
	}

	void WindowsCtrlDispatcher::stopService()
	{
		if (serviceQuitLock.available() > 0)
			return;		// Already requested a stop

		serviceQuitLock.release();
		wait();
	}

private:
	QString serviceName;
	QSemaphore serviceStartLock;
	QSemaphore serviceQuitLock;
	SERVICE_TABLE_ENTRY dispatchTable[2];
	SERVICE_STATUS_HANDLE serviceStatusHandle;
	SERVICE_STATUS serviceStatus;

private:
	static WindowsCtrlDispatcher * instance;
};

WindowsCtrlDispatcher * WindowsCtrlDispatcher::instance = NULL;

// --------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- WinAPI callbacks ------------------------------------------------------------------------------------------------------------------------------ //
// --------------------------------------------------------------------------------------------------------------------------------------------------- //

VOID WINAPI ServiceMain(DWORD, LPTSTR *)
{
	WindowsCtrlDispatcher * const ctrl = WindowsCtrlDispatcher::instance;

	// Just register the handler and that's all
	ctrl->serviceStatusHandle = RegisterServiceCtrlHandlerEx(ctrl->dispatchTable[0].lpServiceName, ServiceControlHandler, ctrl);
	if (!ctrl->serviceStatusHandle)  {
		qDaemonLog(QStringLiteral("Couldn't register the service control handler (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
		ctrl->serviceStartLock.release();
		return;
	}

	// Fill up initial values
	ctrl->serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ctrl->serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	ctrl->serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PRESHUTDOWN | SERVICE_ACCEPT_SHUTDOWN;

	// Report initial status to the SCM
	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, serviceWaitHint);

	// Nothing to initialize, so report running
	ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

	ctrl->serviceStartLock.release();		// Free up the main thread
	ctrl->serviceQuitLock.acquire();		// Signalled (released) when we need to quit. Block for now

	ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

DWORD WINAPI ServiceControlHandler(DWORD control, DWORD, LPVOID, LPVOID context)
{
	switch (control)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_PRESHUTDOWN:
	case SERVICE_CONTROL_SHUTDOWN:
		ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, serviceWaitHint);
		QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
		return NO_ERROR;
	case SERVICE_CONTROL_INTERROGATE:
		{
			WindowsCtrlDispatcher * const ctrl = reinterpret_cast<WindowsCtrlDispatcher *>(context);
			ReportServiceStatus(ctrl->serviceStatus.dwCurrentState, ctrl->serviceStatus.dwWin32ExitCode, ctrl->serviceStatus.dwWaitHint);
		}
		return NO_ERROR;
	case SERVICE_CONTROL_CONTINUE:
	case SERVICE_CONTROL_PAUSE:
	default:
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

VOID WINAPI ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	WindowsCtrlDispatcher * const ctrl = WindowsCtrlDispatcher::instance;

	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.
	ctrl->serviceStatus.dwCurrentState = dwCurrentState;
	ctrl->serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ctrl->serviceStatus.dwWaitHint = dwWaitHint;
	ctrl->serviceStatus.dwControlsAccepted = dwCurrentState == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP;
	ctrl->serviceStatus.dwCheckPoint = dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED ? 0 : dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(ctrl->serviceStatusHandle, &ctrl->serviceStatus);
}

// --------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- DaemonBackendWindows -------------------------------------------------------------------------------------------------------------------------- //
// --------------------------------------------------------------------------------------------------------------------------------------------------- //

DaemonBackendWindows::DaemonBackendWindows(QCommandLineParser & parser)
	: QAbstractDaemonBackend(parser)
{
}

DaemonBackendWindows::~DaemonBackendWindows()
{
}

int DaemonBackendWindows::exec()
{
	QStringList arguments = parser.positionalArguments();
	arguments.prepend(QDaemonApplication::applicationFilePath());

	WindowsCtrlDispatcher dispatcher(QDaemonApplication::applicationName());
	dispatcher.startService();

	QMetaObject::invokeMethod(qApp, "daemonized", Qt::QueuedConnection, Q_ARG(QStringList, arguments));
	int status = QCoreApplication::exec();

	dispatcher.stopService();

	return status;
}

QT_END_NAMESPACE
