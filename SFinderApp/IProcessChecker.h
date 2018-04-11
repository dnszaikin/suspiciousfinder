#pragma once

#include "ProcessInfo.h"

class IProcessChecker {
private:
	//rule name
	std::wstring _rule_name;

	//short info about checker
	std::wstring _rule_info;

protected:
	//child class must override this function to talk about its self
	virtual std::wstring get_about() const = 0;

public:
	IProcessChecker() = delete;
	IProcessChecker(const IProcessChecker&) = default;

	IProcessChecker(std::wstring&& rule_name, std::wstring&& rule_info) noexcept 
		: _rule_name(move(rule_name))
		, _rule_info(move(rule_info)) 
	{
	}

	//return true if a process is suspicious
	virtual bool check_process(ProcessInfo&) const = 0;

	//return detailed information about checking rule
	std::wstring about() const {
		return _rule_name + L" : " + get_about();
	}

	//return rule name
	const std::wstring& get_rule_name() const noexcept {
		return _rule_name;
	}

	const std::wstring& get_rule_info() const noexcept {
		return _rule_info;
	}
	virtual ~IProcessChecker() {};
};