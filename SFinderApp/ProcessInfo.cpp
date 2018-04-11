#include "stdafx.h"
#include <iostream>
#include "ProcessInfo.h"
#include "custom_exception.h"

using namespace std;

ProcessInfo::ProcessInfo(DWORD pid, wstring&& exe_name, wstring&& exe_path, vector<MODULEENTRY32W>&& modules) noexcept :
	_pid(pid),
	_exe_name(move(exe_name)),
	_exe_path(move(exe_path)),
	_modules_information(move(modules)),
	_failure_info({})
{
}

ProcessInfo::~ProcessInfo()
{

}

bool ProcessInfo::is_suspicious() const 
{
	return !_failure_info.empty();
}

DWORD ProcessInfo::get_pid() const {
	return _pid;
}

void ProcessInfo::add_module_failure_info(const std::wstring& checker, const std::wstring& str)
{
	_failure_info.emplace_back(L"Rule: " + checker + L", failed module: " + str);
}

void ProcessInfo::add_failure_info(const wstring& str) {
	_failure_info.emplace_back(str);
}

vector<std::wstring> ProcessInfo::get_failure_info() const {
	return _failure_info;
}

const std::wstring& ProcessInfo::get_exe_name() const
{
	return _exe_name;
}

const std::wstring& ProcessInfo::get_exe_path() const
{
	return _exe_path;
}

const std::vector<MODULEENTRY32W>& ProcessInfo::get_modules() const
{
	return _modules_information;
}