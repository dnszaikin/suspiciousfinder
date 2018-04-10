#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

class ProcessInfo {
private:
	DWORD _pid;
	std::wstring _exe_name;
	std::wstring _exe_path;

	std::vector<MODULEENTRY32W> _modules_information;
	std::vector<std::wstring> _failure_info;

public:

	ProcessInfo() = delete;
	ProcessInfo(const ProcessInfo&) = delete;

	ProcessInfo(ProcessInfo&&) = default;

	ProcessInfo(DWORD pid, std::wstring&& exe_name, std::wstring&& exe_path, std::vector<MODULEENTRY32W>&& modules);

	virtual ~ProcessInfo();

	bool is_suspicious() const ;

	DWORD get_pid() const;

	void add_module_failure_info(const std::wstring& checker, const std::wstring& str);

	void add_failure_info(const std::wstring& str);

	std::vector<std::wstring> get_failure_info() const;

	const std::wstring& get_exe_name() const;
	const std::wstring& get_exe_path() const;
	const std::vector<MODULEENTRY32W>& get_modules() const;
};