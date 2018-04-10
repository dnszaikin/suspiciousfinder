#pragma once

#include "IProcessChecker.h"

class DummyChecker : public IProcessChecker {
private:
	std::wstring _error_info;

public:
	DummyChecker() :_error_info(L"PID of this process is less than 1000") {
		_rule_name = L"DummyChecker";
	}

	bool check_process(ProcessInfo& process) const override {
		return (process.get_pid() < 1000);
	}

	std::wstring get_info() const override {
		return _error_info;
	}

	std::wstring about() const override {
		return _rule_name + L": if PID less 1000 than process is suspicious";
	}

};

