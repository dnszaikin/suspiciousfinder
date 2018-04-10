#pragma once

#include <Windows.h>
#include <msi.h>
#include <string>
#include <iostream>
#include <set>
#include "Shlwapi.h"

#pragma comment(lib, "msi.lib")
#pragma comment(lib, "shlwapi.lib")

namespace installation_info {

#define NORMALIZE_PATH(buff) PathUnquoteSpaces(buff); GetLongPathName(buff, buff, MAX_PATH); wcscpy_s(buff, CharLower(buff));

	typedef std::set<std::wstring> folders_t;

	static void _msi_query_property(LPCTSTR szProductCode, LPCTSTR szUserSid, MSIINSTALLCONTEXT dwContext, LPCTSTR szProperty, std::wstring& value) {

		DWORD cchValue = 0;

		UINT ret2 = MsiGetProductInfoEx(szProductCode, szUserSid, dwContext, szProperty, NULL, &cchValue);

		if (ret2 == ERROR_SUCCESS) {
			cchValue++;
			value.resize(cchValue);

			wchar_t buff[MAX_PATH];

			ret2 = MsiGetProductInfoEx(szProductCode, szUserSid, dwContext, szProperty, (LPTSTR)&buff, &cchValue);

			NORMALIZE_PATH(buff)

			value.assign(buff);
		}
	}

	//get installation folders using MSI API
	static void get_msi_installation_folders(folders_t& folders) {
		UINT ret = 0;
		DWORD dwIndex = 0;
		TCHAR szInstalledProductCode[39] = { 0 };
		TCHAR szSid[128] = { 0 };
		DWORD cchSid;
		MSIINSTALLCONTEXT dwInstalledContext;

		do {
			memset(szInstalledProductCode, 0, sizeof(szInstalledProductCode));
			cchSid = sizeof(szSid) / sizeof(szSid[0]);

			ret = MsiEnumProductsEx(NULL, L"s-1-1-0", MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE, dwIndex,
				szInstalledProductCode,	&dwInstalledContext, szSid,	&cchSid);

			if (ret == ERROR_SUCCESS) {
				std::wstring location;

				_msi_query_property(szInstalledProductCode, cchSid == 0 ? NULL : szSid, dwInstalledContext, INSTALLPROPERTY_INSTALLLOCATION, location);

				if (!location.empty()) {					
					folders.emplace(move(location));
				}

				dwIndex++;
			}

		} while (ret == ERROR_SUCCESS);
	}

	static void _registry_query_value(HKEY hKey, LPCTSTR szName, std::wstring& value)
	{
		DWORD dwType;
		DWORD dwSize = 0;

		if (RegQueryValueEx(hKey, szName, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS && dwSize > 0) {

			wchar_t buff[MAX_PATH];

			RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE)&buff, &dwSize);

			NORMALIZE_PATH(buff)

			value.assign(buff);
		}
	}

	static void _registry_enum(HKEY hKey, folders_t& folders)
	{
		DWORD dwIndex = 0;
		DWORD cbName = 1024;
		TCHAR szSubKeyName[1024];

		LONG ret = ERROR_SUCCESS;

		while (true) {

			cbName = 1024;
			ret = RegEnumKeyEx(hKey, dwIndex, szSubKeyName, &cbName, NULL, NULL, NULL, NULL);

			if (ret == ERROR_NO_MORE_ITEMS) {
				break;
			}

			if (ret == ERROR_SUCCESS) {

				HKEY hItem;
				if (RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hItem) != ERROR_SUCCESS) {
					continue;
				}

				std::wstring location;

				_registry_query_value(hItem, L"InstallLocation", location);

				if (!location.empty()) {
					folders.emplace(move(location));
				}

				RegCloseKey(hItem);
			}

			dwIndex++;
		}

		RegCloseKey(hKey);
	}

	static void _registry_enum(const std::wstring& key, folders_t& folders)
	{
		HKEY hKey;

		LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

		if (ret == ERROR_SUCCESS) {
			_registry_enum(hKey, folders);
		}

		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ | KEY_WOW64_32KEY, &hKey);

		if (ret == ERROR_SUCCESS) {
			_registry_enum(hKey, folders);
		}
	}

	static void get_registry_installation_folders(folders_t& folders) {
		_registry_enum(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", folders);
	}
}