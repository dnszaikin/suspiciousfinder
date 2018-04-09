#pragma once

#include <iostream>
#include <string>
#include <locale> 
#include <codecvt>

namespace utils {

	static std::wstring string_to_wide(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	static std::string wide_to_string(const std::wstring& wstr)
	{
		std::string str{ "" };

		try {
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.to_bytes(wstr);
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		return str;
	}

	static std::wstring format_error_wstr(DWORD err)
	{
		wchar_t* msgbuf = NULL;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    //default language
			(LPWSTR)&msgbuf,
			0,
			NULL);

		if (!msgbuf) {
			wsprintf(msgbuf, L"%i", err);
		}

		std::wstring str(msgbuf);

		LocalFree(msgbuf);

		return str;
	}

	static std::string format_error_str(DWORD err)
	{
		return wide_to_string(format_error_wstr(err));
	}

	static BOOL _SetPrivilege(
		HANDLE hToken,  // token handle 
		LPCTSTR Privilege,  // Privilege to enable/disable 
		BOOL bEnablePrivilege  // TRUE to enable. FALSE to disable 
	)
	{
		TOKEN_PRIVILEGES tp = { 0 };
		// Initialize everything to zero 
		LUID luid;
		DWORD cb = sizeof(TOKEN_PRIVILEGES);
		if (!LookupPrivilegeValue(NULL, Privilege, &luid))
			return FALSE;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if (bEnablePrivilege) {
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		}
		else {
			tp.Privileges[0].Attributes = 0;
		}
		AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL);
		if (GetLastError() != ERROR_SUCCESS)
			return FALSE;

		return TRUE;
	}

	//setting debug privileges to get access to some processes
	static void enable_debug_privileges() 
	{
		HANDLE hToken;

		if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) { 
			if (GetLastError() == ERROR_NO_TOKEN) {
				if (!ImpersonateSelf(SecurityImpersonation)) {
					return;
				}

				if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {
					return;
				}
			} else {
				return;
			}
		}

		// enable SeDebugPrivilege to get access to some processes
		if (!_SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
			CloseHandle(hToken);
		}
	}
}
