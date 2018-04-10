#pragma once

#include "ProcessInfo.h"

class IProcessChecker {
protected:
	std::wstring _rule_name;

public:
	//return true if a process is suspicious
	virtual bool check_process(ProcessInfo&) const = 0;

	//return information about checking rule
	virtual std::wstring get_info() const = 0;
	virtual std::wstring about() const = 0;

	std::wstring get_rule_name() const {
		return _rule_name;
	}

	virtual ~IProcessChecker() {};
};