#pragma once

#include "IProcessChecker.h"

class DummyChecker : public IProcessChecker {
private:

public:
	DummyChecker():IProcessChecker::IProcessChecker(L"DummyChecker", L"PID of this process is less than 1000")  {
	}

	bool check_process(ProcessInfo& process) const override {
		return (process.get_pid() < 1000);
	}

	std::wstring get_about() const override {
		return L"If PID less 1000 than process is suspicious";
	}

};

