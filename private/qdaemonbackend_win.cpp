#include "qdaemonbackend_win.h"
#include "qdaemonlog.h"
#include "qdaemonapplication.h"

#include <QFileInfo>
#include <QElapsedTimer>

static const qint32 serviceWaitHint = 1000;			// Just a service control sugar (1 second)

WindowsCtrlDispatcher * WindowsCtrlDispatcher::instance = NULL;

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

WindowsCtrlDispatcher::WindowsCtrlDispatcher()
	: QThread(NULL), serviceStartLock(0), serviceQuitLock(0), serviceStatusHandle(NULL)
{
	instance = this;
	::memset(dispatchTable, 0, 2 * sizeof(SERVICE_TABLE_ENTRY));
}

void WindowsCtrlDispatcher::run()
{
	dispatchTable[0].lpServiceName = reinterpret_cast<LPTSTR>(const_cast<ushort *>(serviceName.utf16()));
	dispatchTable[0].lpServiceProc = ServiceMain;

	// Just call the control dispatcher and hang
	if (!StartServiceCtrlDispatcher(dispatchTable))  {
		qDaemonLog(QStringLiteral("Couldn't run the service control dispatcher (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
		serviceStartLock.release();		// Free the main thread
	}

	QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);	// We want to quit (if we are not already quitting)
}

void WindowsCtrlDispatcher::stopService()
{
	if (serviceQuitLock.available() > 0)
		return;		// Already requested a stop

	serviceQuitLock.release();
	wait();
}

void WindowsCtrlDispatcher::startService(const QString & name)
{
	serviceName = name;
	start();						// Start the control thread
	serviceStartLock.acquire();		// Wait until the service has started
}

// ------------------------------------------------------------------------------------------------------ //

BackendWindows::BackendWindows()
{
}

BackendWindows::~BackendWindows()
{
}

bool BackendWindows::initialize()
{
	name = QDaemonApplication::applicationName();
	path = QFileInfo(QDaemonApplication::applicationFilePath()).absoluteFilePath();
	description = QDaemonApplication::applicationDescription();

	if (name.size() > 256)  {	// Max length for service names is 256 chars - https://msdn.microsoft.com/en-us/library/windows/desktop/ms682450(v=vs.85).aspx
		qDaemonLog(QStringLiteral("Service name too long (exceeded 256 characters)."), QDaemonLog::ErrorEntry);
		return false;
	}

	return true;
}

bool BackendWindows::finalize()
{
	return true;
}

QDaemonBackend * BackendWindows::create(BackendType type)
{
	switch (type)
	{
	case QDaemonBackend::ControllerType:
		return new ControllerBackendWindows();
	case QDaemonBackend::DaemonType:
		return new DaemonBackendWindows();
	default:
		Q_ASSERT(false);
		return nullptr;
	}
}

DaemonBackendWindows::DaemonBackendWindows()
{
}

DaemonBackendWindows::~DaemonBackendWindows()
{
}

bool DaemonBackendWindows::initialize()
{
	if (!BackendWindows::initialize())
		return false;

	dispatcher.startService(name);
	return true;
}

bool DaemonBackendWindows::finalize()
{
	dispatcher.stopService();
	return BackendWindows::finalize();
}

bool DaemonBackendWindows::start()
{
	Q_ASSERT(false);
	return false;
}

bool DaemonBackendWindows::stop()
{
	Q_ASSERT(false);
	return false;
}

bool DaemonBackendWindows::install()
{
	Q_ASSERT(false);
	return false;
}

bool DaemonBackendWindows::uninstall()
{
	Q_ASSERT(false);
	return false;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------- //
// --- ControllerBackendWindows ------------------------------------------------------------------------------------------------------------------------------ //
// -------------------------------------------------------------------------------------------------------------------------------------------------------- //

static const qint32 serviceNotifyTimeout = 30000;			// Up to 30 seconds

ControllerBackendWindows::ControllerBackendWindows()
	: manager(NULL)
{
}

ControllerBackendWindows::~ControllerBackendWindows()
{
}

bool ControllerBackendWindows::initialize()
{
	if (!BackendWindows::initialize())
		return false;

	// Open a handle to the service manager
	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!manager)  {
		qDaemonLog(QStringLiteral("Couldn't open a handle to the service manager (Error code %1)").arg(GetLastError()), QDaemonLog::ErrorEntry);
		return false;
	}

	return true;
}

bool ControllerBackendWindows::finalize()
{
	bool ok = true;
	if (!CloseServiceHandle(manager))  {
		qDaemonLog(QStringLiteral("Error while closing the service manager (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);
		ok = false;
	}

	return BackendWindows::finalize() && ok; // Order is important - left associativity!
}

bool ControllerBackendWindows::start()
{
	SC_HANDLE service = openService(SERVICE_START | SERVICE_QUERY_STATUS);
	if (!service)
		return false;

	// Start the service
	if (!StartService(service, 0, NULL))  {
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			qDaemonLog(QStringLiteral("Couldn't stop the service: Access denied."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_PATH_NOT_FOUND:
			qDaemonLog(QStringLiteral("Couldn't start the service: The path to the executable is set incorrectly."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_SERVICE_ALREADY_RUNNING:
			qDaemonLog(QStringLiteral("The service is already running."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_SERVICE_REQUEST_TIMEOUT:
			qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
			break;
		default:
			qDaemonLog(QStringLiteral("Couldn't start the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
		}

		closeService(service);
		return false;
	}

	bool ok = waitForStatus(service, SERVICE_RUNNING);
	closeService(service);
	return ok;
}

bool ControllerBackendWindows::stop()
{
	SC_HANDLE service = openService(SERVICE_STOP | SERVICE_QUERY_STATUS);
	if (!service)
		return false;

	// Stop the service
	SERVICE_STATUS serviceStatus;
	if (!ControlService(service, SERVICE_CONTROL_STOP, &serviceStatus))  {
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			qDaemonLog(QStringLiteral("Couldn't stop the service, access denied."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_SERVICE_NOT_ACTIVE:
			qDaemonLog(QStringLiteral("The service is not running."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_SERVICE_REQUEST_TIMEOUT:
			qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
			break;
		default:
			qDaemonLog(QStringLiteral("Couldn't stop the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
		}

		closeService(service);
		return false;
	}

	bool ok = waitForStatus(service, SERVICE_STOPPED);
	closeService(service);
	return ok;
}

bool ControllerBackendWindows::install()
{
	SC_HANDLE service = createService();
	if (!service)
		return false;

	// Set the description if provided
	if (!description.isEmpty())  {
		LPTSTR lpDescription = reinterpret_cast<LPTSTR>(const_cast<ushort *>(description.utf16()));		// Sadly, MS doesn't care about constness

		SERVICE_DESCRIPTION serviceDescription;
		serviceDescription.lpDescription = lpDescription;
		if (!ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
			qDaemonLog(QStringLiteral("Couldn't set the service description (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);
	}

	// TODO: Add the service location to the PATH

	closeService(service);
	return true;
}

bool ControllerBackendWindows::uninstall()
{
	SC_HANDLE service = openService(DELETE);
	if (!service)
		return false;

	// TODO: Remove the service location from the PATH

	// Delete the service
	if (!DeleteService(service))  {
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_SERVICE_MARKED_FOR_DELETE:
			qDaemonLog(QStringLiteral("The service had already been marked for deletion."), QDaemonLog::ErrorEntry);
			break;
		default:
			qDaemonLog(QStringLiteral("Couldn't delete the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
		}

		closeService(service);
		return false;
	}

	closeService(service);
	return true;
}

bool ControllerBackendWindows::waitForStatus(SC_HANDLE service, int requestedStatus)
{
	const unsigned long pollInterval = serviceNotifyTimeout / 20;		// A twentieth of the allowed timeout interval (in ms)

	// Do polling (Windows XP compatible)
	SERVICE_STATUS_PROCESS status;
	DWORD bytesNeeded;

	QElapsedTimer timeout;
	timeout.start();
	while (!timeout.hasExpired(serviceNotifyTimeout))  {
		if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded))  {
			qDaemonLog(QStringLiteral("Couldn't obtain the service status (Error code %1).").arg(GetLastError()), QDaemonLog::ErrorEntry);
			return false;
		}

		if (status.dwCurrentState == requestedStatus)
			return true;

		// Wait a bit before trying out
		QThread::msleep(pollInterval);
	}

	qDaemonLog(QStringLiteral("The service is not responding."), QDaemonLog::ErrorEntry);
	return false;
}

SC_HANDLE ControllerBackendWindows::createService()
{
	LPCTSTR lpName = reinterpret_cast<LPCTSTR>(name.utf16());
	LPCTSTR lpPath = reinterpret_cast<LPCTSTR>(path.utf16());

	// Create the service
	SC_HANDLE service = CreateService(manager, lpName, lpName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, lpPath, NULL, NULL, NULL, NULL, NULL);
	if (!service)  {
		DWORD error =  GetLastError();
		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			qDaemonLog(QStringLiteral("Couldn't install the service, access denied."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
		case ERROR_SERVICE_EXISTS:
			qDaemonLog(QStringLiteral("The service was already installed (Uninstall first)."), QDaemonLog::ErrorEntry);
			break;
		default:
			qDaemonLog(QStringLiteral("Couldn't install the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
		}
	}

	return service;
}

SC_HANDLE ControllerBackendWindows::openService(int access)
{
	LPCTSTR lpName = reinterpret_cast<LPCTSTR>(name.utf16());

	// Open the service
	SC_HANDLE service = OpenService(manager, lpName, access);
	if (!service)  {
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			qDaemonLog(QStringLiteral("Couldn't open service control, access denied."), QDaemonLog::ErrorEntry);
			break;
		case ERROR_SERVICE_DOES_NOT_EXIST:
			qDaemonLog(QStringLiteral("The service is not installed."), QDaemonLog::ErrorEntry);
			break;
		default:
			qDaemonLog(QStringLiteral("Couldn't open the service (Error code %1).").arg(error), QDaemonLog::ErrorEntry);
		}
	}

	return service;
}

void ControllerBackendWindows::closeService(SC_HANDLE service)
{
	if (service && !CloseServiceHandle(service))
		qDaemonLog(QStringLiteral("Error while closing the service handle (Error code %1).").arg(GetLastError()), QDaemonLog::WarningEntry);
}
