#pragma once
#include <string>
#include "ProcessInfo.h"
#include "IProcessChecker.h"

class SignatureChecker : public IProcessChecker
{
private:
	std::wstring _error_info;

public:
	SignatureChecker();
	virtual ~SignatureChecker();

	bool check_process(ProcessInfo&) const override;

	std::wstring get_info() const override;

	std::wstring about() const override {
		return _rule_name + L": check if a process and its modules are singned";
	}
};


