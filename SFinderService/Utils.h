#pragma once
#include "stdafx.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <locale> 
#include <codecvt>

namespace utils {

	static std::wstring string_to_wide(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	static std::string wide_to_string(const std::wstring& wstr)
	{
		std::string str{ "" };

		try {
			using convert_typeX = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_typeX, wchar_t> converterX;

			return converterX.to_bytes(wstr);
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		return str;
	}

	static std::wstring format_error_wstr(DWORD err)
	{
		wchar_t* msgbuf = NULL;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,               
			err,               
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    //default language
			(LPWSTR)&msgbuf,
			0,     
			NULL);               

		if (!msgbuf) {
			wsprintf(msgbuf, L"%i", err);
		}
		
		std::wstring str(msgbuf);

		LocalFree(msgbuf);

		return str;
	}

	static std::string format_error_str(DWORD err)
	{
		return wide_to_string(format_error_wstr(err));
	}
}