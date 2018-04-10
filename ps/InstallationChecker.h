#pragma once
#include "IProcessChecker.h"
#include "installation_info.h"

class TrustedFoldersChecker : public IProcessChecker
{
private:
	std::wstring _error_info;
	installation_info::folders_t _trusted_folders;
	void add_system_directory(wchar_t*);
public:
	TrustedFoldersChecker();
	virtual ~TrustedFoldersChecker();

	bool check_process(ProcessInfo&) const override;
	std::wstring get_info() const override;

	std::wstring about() const override {

		std::wstring str;

		for (auto&& folder : _trusted_folders) {
			str += folder + L"\n";
		}

		return _rule_name + L": check if process and its modules are running from trusted folders:\n" + str;
	}
};

