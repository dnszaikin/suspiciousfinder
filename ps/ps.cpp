// ps.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdlib.h>

#include <Objbase.h>
#include <atlbase.h>
#include <shobjidl.h>
#include <knownfolders.h>
#include <propkey.h>
#include <shlguid.h>

#include "Utils.h"
#include "ProcessInfo.h"
#include "process_info.h"

using namespace std;

class IProcessChecker {
public:
	//return true if a process is suspicious
	virtual bool check_process(ProcessInfo&) const  = 0;

	//return information about checking rule
	virtual wstring get_info() const = 0;

	virtual ~IProcessChecker() {};
};

class DummyChecker : public IProcessChecker {
private: 
	wstring _error_info;

public:
	DummyChecker():_error_info(L"PID of this process is less than 1000") {

	}

	bool check_process(ProcessInfo& process) const override {
		
		return (process.get_pid() < 1000);
	}
	
	wstring get_info() const override {
		return _error_info;
	}
};

class TrustedFolderChecker : public IProcessChecker {
private:
	wstring _error_info;
	vector<wstring> _trusted_folders;

public:
	TrustedFolderChecker() {
		wchar_t windir[MAX_PATH];

		GetWindowsDirectory(windir, MAX_PATH);

		_trusted_folders.emplace_back(windir);

		_error_info = L"This process are contains modules from the folders other than systems: ";

		for (auto&& folder : _trusted_folders) {
			_error_info += folder + L"\n";
		}
	}

	bool check_process(ProcessInfo& process) const override {
		size_t _count = process.get_failure_info().size();

		bool error_info_added = false;

		for (auto&& module : process.get_modules()) {
			for (auto&& folder : _trusted_folders) {			
				if (_wcsicmp(module.szExePath, folder.c_str()) < 0) {

					if (!error_info_added) {
						process.add_failure_info(_error_info);
						error_info_added = true;
					}

					process.add_failure_info(L"\t" + wstring(module.szExePath));
				}
			}
		}

		return process.get_failure_info().size() != _count;
	}

	wstring get_info() const override {
		return _error_info;
	}
};

class InstallationChecker : public IProcessChecker {
private:
	wstring _error_info;
	vector<wstring> _trusted_folders;

public:
	InstallationChecker() {
		wchar_t windir[MAX_PATH];

		GetWindowsDirectory(windir, MAX_PATH);

		_trusted_folders.emplace_back(windir);

		_error_info = L"This process are contains modules from the folders other than systems: ";

		for (auto&& folder : _trusted_folders) {
			_error_info += folder + L"\n";
		}
	}

	bool check_process(ProcessInfo& process) const override {
		size_t _count = process.get_failure_info().size();

		bool error_info_added = false;

		for (auto&& module : process.get_modules()) {
			for (auto&& folder : _trusted_folders) {
				if (_wcsicmp(module.szExePath, folder.c_str()) < 0) {

					if (!error_info_added) {
						process.add_failure_info(_error_info);
						error_info_added = true;
					}

					process.add_failure_info(L"\t" + wstring(module.szExePath));
				}
			}
		}

		return process.get_failure_info().size() != _count;
	}

	wstring get_info() const override {
		return _error_info;
	}
};

class ProcessCheckerVisitor {
private:
	vector<unique_ptr<IProcessChecker>> _checkers;

public:

	void add_checker(unique_ptr<IProcessChecker>&& checker) {
		_checkers.emplace_back(move(checker));
	}

	void check_process(ProcessInfo& process) const {
		for (auto&& checker : _checkers) {
			checker->check_process(process);
		}
	}

	virtual ~ProcessCheckerVisitor()
	{

	}
};


class CCoInitialize {
public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) { }
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

//
//void PrintDetail(IShellItem2 *psi,
//	const SHCOLUMNID *pscid, PCTSTR pszLabel)
//{
//	PROPVARIANT vt;
//	HRESULT hr = psi->GetProperty(*pscid, &vt);
//	if (SUCCEEDED(hr)) {
//		hr = VariantChangeType(&vt, &vt, 0, VT_BSTR);
//		if (SUCCEEDED(hr)) {
//			_tprintf(TEXT("%s: %ws\n"), pszLabel, V_BSTR(&vt));
//		}
//		PropVariantClear(&vt);
//	}
//}

void PrintDetail(IShellItem2 *psi2,
	const SHCOLUMNID *pscid, PCTSTR pszLabel)
{
	LPWSTR pszValue;
	HRESULT hr = psi2->GetString(*pscid, &pszValue);
	if (SUCCEEDED(hr)) {
		_tprintf(TEXT("%s: %ws\n"), pszLabel, pszValue);
		CoTaskMemFree(pszValue);
	}
}

int wmain(void)
{
	
	CCoInitialize init;

	CComPtr<IShellItem> spPrograms;

	SHCreateItemFromParsingName(
		L"::{26EE0668-A00A-44D7-9371-BEB064C98683}\\8\\"
		L"::{7B81BE6A-CE2B-4676-A29E-EB907A5126C5}", nullptr,
		IID_PPV_ARGS(&spPrograms));

	CComPtr<IEnumShellItems> spEnum;

	spPrograms->BindToHandler(nullptr, BHID_EnumItems,
		IID_PPV_ARGS(&spEnum));

	for (CComPtr<IShellItem> spProgram;
		spEnum->Next(1, &spProgram, nullptr) == S_OK;
		spProgram.Release()) {
		CComHeapPtr<wchar_t> spszName;
		spProgram->GetDisplayName(SIGDN_NORMALDISPLAY, &spszName);
		wprintf(L"%ls\n", spszName);
		PrintDetail(CComQIPtr<IShellItem2>(spProgram), &PKEY_Size, L"Size");
	}
	return 0;
	

	try {
		utils::enable_debug_privileges();

		vector<ProcessInfo> processes;

		process_info::get_process_information(processes);

		cout << "Total process running in system: " << processes.size() << endl;

		cout << "Looking for suspicious..." << endl;

		auto checker = ProcessCheckerVisitor();

		checker.add_checker(make_unique<DummyChecker>(DummyChecker()));
		checker.add_checker(make_unique<TrustedFolderChecker>(TrustedFolderChecker()));

		for (auto&& process: processes) {

			checker.check_process(process);

			if (process.is_suspicious()) {
				cout << "==================================================================================" << endl;
				wcout << process.get_exe_name() << " (" << process.get_pid() << ") is marked as suspicious" << endl;

				if (!process.get_exe_path().empty()) {
					wcout << process.get_exe_path() << endl;
				}

				cout << "The following rules are failed: " << endl;

				for (auto&& str : process.get_failure_info()) {
					wcout << L"\t" << str << endl;
				}
				cout << "==================================================================================" << endl;
			}
		}
	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
	catch (...) {
		cerr << "Unknown error" << endl;
	}

	return 0;
}


