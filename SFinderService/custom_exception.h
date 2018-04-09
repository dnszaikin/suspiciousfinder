#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "Utils.h"



class custom_exception : public std::exception
{
private:
	std::string _error_str;
	int _err_code;
	bool _err_code_present;

public:
	custom_exception(const std::wstring& msg) : _error_str(utils::wide_to_string(msg)), _err_code(0), _err_code_present(false)
	{

	};

	custom_exception(const std::wstring& msg, int err_code) : _error_str(utils::wide_to_string(msg)), _err_code(err_code), _err_code_present(true)
	{

	};

	custom_exception(const std::string& msg) : _error_str(msg), _err_code(0), _err_code_present(false)
	{

	};

	custom_exception(const std::string& msg, int err_code) : _error_str(msg), _err_code(err_code), _err_code_present(true)
	{

	};
	const char* what() const throw() 
	{ 
		if (_err_code_present) {
			std::stringstream ss;
	
			ss << _error_str << ". Error message: " << utils::format_error_str(_err_code) << " Error code: " << _err_code;

			return _strdup(ss.str().c_str());
		}

		return _error_str.c_str();
	}
};



