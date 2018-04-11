#include "stdafx.h"
#include "SignatureChecker.h"
#include "signature_info.h"

using namespace std;

SignatureChecker::SignatureChecker() :IProcessChecker::IProcessChecker(L"Singature checker", L"This process contains unsigned or not valid signed modules.")
{
}

SignatureChecker::~SignatureChecker()
{
}

bool SignatureChecker::check_process(ProcessInfo& process) const {
	wstring info;

	bool result = sing_info::VerifyEmbeddedSignature(process.get_exe_path(), info);

	if (!result) {
		process.add_module_failure_info(get_rule_name(), move(info));
	}

	return !result;
}
