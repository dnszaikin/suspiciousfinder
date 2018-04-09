#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>

#include "Utils.h"
#include "custom_exception.h"
#include "ProcessInfo.h"

using namespace std;

namespace process_info {

	//return list of loaded modules for specified dwPID
	static void ListProcessModules(DWORD dwPID, std::vector<MODULEENTRY32W>& vec)
	{
		HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
		MODULEENTRY32 me32;

		hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPID);

		if (hModuleSnap == INVALID_HANDLE_VALUE) {
			return;
		}

		// Set the size of the structure before using it.
		me32.dwSize = sizeof(MODULEENTRY32);

		if (!Module32First(hModuleSnap, &me32))
		{
			CloseHandle(hModuleSnap);
			return;
		}

		//loading collection
		do {
			vec.emplace_back(move(me32));

		} while (Module32Next(hModuleSnap, &me32));

		CloseHandle(hModuleSnap);
	}

	//get all process and its modules, throw excpetion
	static void get_process_information(std::vector<ProcessInfo>& process_info)
	{
		HANDLE hProcessSnap;
		HANDLE hProcess;
		PROCESSENTRY32 pe32;
		DWORD dwPriorityClass;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			throw custom_exception("Unable to take a snapshot of all runnig processes", GetLastError());
			return;
		}

		pe32.dwSize = sizeof(PROCESSENTRY32);


		if (!Process32First(hProcessSnap, &pe32)) {
			CloseHandle(hProcessSnap);          // clean the snapshot object
			throw custom_exception("Unable to get process information", GetLastError());
		}

		do {
			vector<MODULEENTRY32W> vec;
			wstring exe_path{};

			ListProcessModules(pe32.th32ProcessID, vec);

			if (!vec.empty()) {
				for (auto&& item : vec) {
					if (item.th32ProcessID == pe32.th32ProcessID) {
						exe_path.assign(item.szExePath);
						break;
					}
				}
			}

			auto pi = ProcessInfo(pe32.th32ProcessID, wstring(pe32.szExeFile), move(exe_path), move(vec));

			process_info.emplace_back(move(pi));

		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);
	}
}
