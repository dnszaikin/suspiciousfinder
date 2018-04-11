#include "stdafx.h"
#include <Shlwapi.h>
#include "TrustedFolderChecker.h"

#pragma comment(lib, "shlwapi.lib")

using namespace std;

TrustedFoldersChecker::TrustedFoldersChecker(): IProcessChecker::IProcessChecker(L"Trusted folder checker", L"This process contains modules from the folders other than trusted folders.")
{
	installation_info::get_msi_installation_folders(_trusted_folders);
	installation_info::get_registry_installation_folders(_trusted_folders);

	wchar_t windir[MAX_PATH];

	GetWindowsDirectory(windir, MAX_PATH);

	wcscpy_s(windir, CharLower(windir));
	PathAddBackslash(windir);

	_system_folder.assign(windir);

	//GetSystemDirectory(windir, MAX_PATH);

	//add_system_directory(windir);

	//GetSystemWow64Directory(windir, MAX_PATH);

	//add_system_directory(windir);

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

		//check if process is running from system folder
		const wchar_t* p = wcsstr(module.szExePath, _system_folder.c_str());

		if (p != nullptr) {
			continue;
		}

		size_t failed_rules = 0;

		for (auto&& folder : _trusted_folders) {
			wstring exe_path(module.szExePath);

			size_t pos = exe_path.find(folder);

			if (pos == wstring::npos) {
				++failed_rules;
			}
			else {
				failed_rules = 0;
				break;
			}
		}

		if (failed_rules > 0) {
			process.add_module_failure_info(get_rule_name(), wstring(module.szExePath));
		}
	}

	return process.get_failure_info().size() != _count;
}
