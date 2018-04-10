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

#include "Utils.h"
#include "ProcessInfo.h"
#include "IProcessChecker.h"
#include "DummyChecker.h"
#include "InstallationChecker.h"
#include "process_info.h"

using namespace std;

class ProcessCheckerVisitor {
private:
	vector<unique_ptr<IProcessChecker>> _checkers;

public:

	void add_checker(unique_ptr<IProcessChecker>&& checker) {
		_checkers.emplace_back(move(checker));
	}

	void check_process(ProcessInfo& process) const {
		for (auto&& checker : _checkers) {
			if (checker->check_process(process)) {
				process.add_failure_info(checker->get_info());
			}
		}
	}

	void about() {
		for (auto&& checker : _checkers) {
			wcout << checker->about() << endl;
		}
	}

	virtual ~ProcessCheckerVisitor()
	{

	}
};

int wmain(void)
{
	try {

		utils::enable_debug_privileges();

		vector<ProcessInfo> processes;

		process_info::get_process_information(processes);

		cout << "Total process running in system: " << processes.size() << endl;

		cout << "Looking for suspicious..." << endl;

		auto checker = ProcessCheckerVisitor();

		//checker.add_checker(make_unique<DummyChecker>(DummyChecker()));
		checker.add_checker(make_unique<TrustedFoldersChecker>(TrustedFoldersChecker()));

		checker.about();

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


