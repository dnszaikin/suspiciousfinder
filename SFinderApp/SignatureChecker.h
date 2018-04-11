#pragma once
#include <string>
#include "ProcessInfo.h"
#include "IProcessChecker.h"

class SignatureChecker : public IProcessChecker
{
private:

public:
	SignatureChecker();
	virtual ~SignatureChecker();

	bool check_process(ProcessInfo&) const override;

	std::wstring get_about() const override {
		return L"Check if a process and its modules are singned";
	}
};


