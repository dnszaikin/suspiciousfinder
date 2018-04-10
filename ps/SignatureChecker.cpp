#include "stdafx.h"
#include "SignatureChecker.h"
#include "signature_info.h"

using namespace std;

SignatureChecker::SignatureChecker() : _error_info(L"This process contains unsigned or not valid signed modules.")
{
	_rule_name = L"Singature checker";
}


SignatureChecker::~SignatureChecker()
{
}

bool SignatureChecker::check_process(ProcessInfo& process) const {
	wstring info;

	bool result = sing_info::VerifyEmbeddedSignature(process.get_exe_path(), info);

	if (!result) {
		process.add_module_failure_info(_rule_name, move(info));
	}

	return !result;
}

wstring SignatureChecker::get_info() const {
	return _error_info;
}