#ifndef QDAEMONBACKEND_WIN_H
#define QDAEMONBACKEND_WIN_H

#include "qdaemonbackend.h"

#include <QThread>
#include <QSemaphore>

#include <Windows.h>

#ifndef UNICODE
#error Enable unicode support for your compiler.
#endif

QT_BEGIN_NAMESPACE

VOID WINAPI ServiceMain(DWORD, LPTSTR *);
DWORD WINAPI ServiceControlHandler(DWORD control, DWORD, LPVOID, LPVOID context);
VOID WINAPI ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

class Q_DAEMON_LOCAL WindowsCtrlDispatcher : public QThread
{
public:
	WindowsCtrlDispatcher();

	void run() override;

	void startService(const QString &);
	void stopService();
private:

	QString serviceName;
	QSemaphore serviceStartLock;
	QSemaphore serviceQuitLock;
	SERVICE_TABLE_ENTRY dispatchTable[2];
	SERVICE_STATUS_HANDLE serviceStatusHandle;
	SERVICE_STATUS serviceStatus;

private:
	static WindowsCtrlDispatcher * instance;

	friend VOID WINAPI ServiceMain(DWORD, LPTSTR *);
	friend DWORD WINAPI ServiceControlHandler(DWORD control, DWORD, LPVOID, LPVOID context);
	friend VOID WINAPI ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
};

// ------------------------------------------------------------------------------------------------------ //

class Q_DAEMON_LOCAL BackendWindows : public QDaemonBackend
{
	Q_DISABLE_COPY(BackendWindows)

public:
	BackendWindows();
	~BackendWindows() override;

	bool initialize() override;
	bool finalize() override;

	static QDaemonBackend * create(BackendType);

protected:
	QString dir, path, name, description;
};

class Q_DAEMON_LOCAL DaemonBackendWindows : public BackendWindows
{
	Q_DISABLE_COPY(DaemonBackendWindows)

public:
	DaemonBackendWindows();
	~DaemonBackendWindows() override;

	bool initialize() override;
	bool finalize() override;

	bool start() override;
	bool stop() override;
	bool install() override;
	bool uninstall() override;

private:
	WindowsCtrlDispatcher dispatcher;
};

class Q_DAEMON_LOCAL ControllerBackendWindows : public BackendWindows
{
	Q_DISABLE_COPY(ControllerBackendWindows)

public:
	ControllerBackendWindows();
	~ControllerBackendWindows() override;

	bool initialize() override;
	bool finalize() override;

	bool start() override;
	bool stop() override;
	bool install() override;
	bool uninstall() override;

private:
	bool waitForStatus(SC_HANDLE, int);

	SC_HANDLE createService();
	SC_HANDLE openService(int);
	void closeService(SC_HANDLE);

	static bool addToSystemPath(const QString &);

private:
	SC_HANDLE manager;
	static LPCTSTR pathRegistryKey;
};

QT_END_NAMESPACE

#endif // QDAEMONBACKEND_WIN_H
