#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>


/*
limitations
- dll may be load without LoadLibrary, and we don't see it
- system may affected by rootkit that hide present from the system
- out application may be infected and functions called from loaded DLL may be interecebed
*/
using namespace std;

constexpr wchar_t LOG_NAME[]{L"%TEMP%\sfinder.log"};

#define LOG(message)  		{fstream fs(LOG_NAME, fstream::out | fstream::app); fs << message << endl; fs.close();}

wchar_t SERVICE_NAME[] { L"Suspicious Finder Service" };

SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE service_status_handler;

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);

int wmain (int argc, char *argv)
{

	SERVICE_TABLE_ENTRY service_table[] = {
		{SERVICE_NAME, ServiceMain},
		{nullptr, nullptr}
	};

	if (!StartServiceCtrlDispatcher(service_table)) {
		LOG("Unable to start service dispatcher. Error: " << GetLastError());
	}

	return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR * lpszArgv)
{
	service_status_handler = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (service_status_handler == 0) {
		LOG("Unable to register service. Error: " << GetLastError());

		return;
	}

	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwCurrentState = SERVICE_START_PENDING;
	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	service_status.dwServiceSpecificExitCode = 0;
	service_status.dwCheckPoint = 0;
	service_status.dwWaitHint = 5000;

	if (!SetServiceStatus(service_status_handler, &service_status)) {
		LOG("Unable to set START_PENDING status. Error: " << GetLastError());
		return;
	}

	service_status.dwCurrentState = SERVICE_RUNNING;
	service_status.dwWin32ExitCode = NO_ERROR;

	if (!SetServiceStatus(service_status_handler, &service_status)) {
		LOG("Unable to set START_RUNNING status. Error: " << GetLastError());
		return;
	}

	//truncate log before starting
	{
		fstream open(LOG_NAME, fstream::out | fstream::trunc);
	}

	LOG("Started!");
	
	while (service_status.dwCurrentState == SERVICE_RUNNING) {
		Sleep(1000);
	}
	
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		{
			service_status.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(service_status_handler, &service_status);
			LOG("Stopped");
		}
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		{
			service_status.dwCurrentState = SERVICE_STOPPED;
			SetServiceStatus(service_status_handler, &service_status);
		}
		break;
	default:
		{
			++service_status.dwCheckPoint;
			SetServiceStatus(service_status_handler, &service_status);
		}
		break;
	}
	return;
}
