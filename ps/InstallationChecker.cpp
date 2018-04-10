#include "stdafx.h"
#include "InstallationChecker.h"
#include "Shlwapi.h"

#pragma comment(lib, "shlwapi.lib")

using namespace std;

TrustedFoldersChecker::TrustedFoldersChecker(): _error_info(L"This process contains modules from the folders other than trusted folders.")
{
	installation_info::get_msi_installation_folders(_trusted_folders);
	installation_info::get_registry_installation_folders(_trusted_folders);

	wchar_t windir[MAX_PATH];

	GetWindowsDirectory(windir, MAX_PATH);

	add_system_directory(windir);

	GetSystemDirectory(windir, MAX_PATH);

	add_system_directory(windir);

	GetSystemWow64Directory(windir, MAX_PATH);

	add_system_directory(windir);


	_rule_name = L"Trusted folder checker";
}

TrustedFoldersChecker::~TrustedFoldersChecker()
{
}

void TrustedFoldersChecker::add_system_directory(wchar_t* folder)
{
	wchar_t dir[MAX_PATH];

	wcscpy_s(dir, CharLower(folder));

	_trusted_folders.emplace(dir);
}

bool TrustedFoldersChecker::check_process(ProcessInfo& process) const {
	size_t _count = process.get_failure_info().size();
	
	for (auto&& module : process.get_modules()) {

		size_t failed_rules = 0;

		for (auto&& folder : _trusted_folders) {
			wchar_t buff[MAX_PATH];

			wcscpy_s(buff, module.szExePath);

			PathRemoveFileSpec(buff);

			wstring exe_path(buff);

			if (exe_path != folder) {
				++failed_rules;
			}
			else {
				failed_rules = 0;
				break;
			}
		}

		if (failed_rules > 0) {
			process.add_module_failure_info(_rule_name, wstring(module.szExePath));
		}
	}

	return process.get_failure_info().size() != _count;
}

wstring TrustedFoldersChecker::get_info() const {
	return _error_info;
}