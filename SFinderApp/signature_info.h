#pragma once

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <string>
#include <iostream>
#include "Utils.h"

#pragma comment (lib, "wintrust")

namespace sing_info {
	//The base of this code is taken from https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa382384(v=vs.85).aspx
	//return TRUE if singature is verified
	static bool VerifyEmbeddedSignature(const std::wstring& source_file, std::wstring& info)
	{
		
		bool result = false;

		if (source_file.empty()) {
			return true;
		}

		try {
			LONG lStatus;
			DWORD dwLastError;

			// Initialize the WINTRUST_FILE_INFO structure.

			WINTRUST_FILE_INFO FileData;
			memset(&FileData, 0, sizeof(FileData));
			FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
			FileData.pcwszFilePath = source_file.c_str();
			FileData.hFile = NULL;
			FileData.pgKnownSubject = NULL;

			/*
			WVTPolicyGUID specifies the policy to apply on the file
			WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

			1) The certificate used to sign the file chains up to a root
			certificate located in the trusted root certificate store. This
			implies that the identity of the publisher has been verified by
			a certification authority.

			2) In cases where user interface is displayed (which this example
			does not do), WinVerifyTrust will check for whether the
			end entity certificate is stored in the trusted publisher store,
			implying that the user trusts content from this publisher.

			3) The end entity certificate has sufficient permission to sign
			code, as indicated by the presence of a code signing EKU or no
			EKU.
			*/

			GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
			WINTRUST_DATA WinTrustData;

			// Initialize the WinVerifyTrust input data structure.

			// Default all fields to 0.
			memset(&WinTrustData, 0, sizeof(WinTrustData));

			WinTrustData.cbStruct = sizeof(WinTrustData);

			// Use default code signing EKU.
			WinTrustData.pPolicyCallbackData = NULL;

			// No data to pass to SIP.
			WinTrustData.pSIPClientData = NULL;

			// Disable WVT UI.
			WinTrustData.dwUIChoice = WTD_UI_NONE;

			// No revocation checking.
			WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;

			// Verify an embedded signature on a file.
			WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

			// Verify action.
			WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;

			// Verification sets this value.
			WinTrustData.hWVTStateData = NULL;

			// Not used.
			WinTrustData.pwszURLReference = NULL;

			// This is not applicable if there is no UI because it changes 
			// the UI to accommodate running applications instead of 
			// installing applications.
			WinTrustData.dwUIContext = 0;

			// Set pFile.
			WinTrustData.pFile = &FileData;

			// WinVerifyTrust verifies signatures as specified by the GUID 
			// and Wintrust_Data.
			lStatus = WinVerifyTrust(
				NULL,
				&WVTPolicyGUID,
				&WinTrustData);

			switch (lStatus)
			{
			case ERROR_SUCCESS:
				/*
				Signed file:
				- Hash that represents the subject is trusted.

				- Trusted publisher without any verification errors.

				- UI was disabled in dwUIChoice. No publisher or
				time stamp chain errors.

				- UI was enabled in dwUIChoice and the user clicked
				"Yes" when asked to install and run the signed
				subject.
				*/
				result = true;
				break;

			case TRUST_E_NOSIGNATURE:
				// The file was not signed or had a signature 
				// that was not valid.

				// Get the reason for no signature.
				dwLastError = GetLastError();
				if (TRUST_E_NOSIGNATURE == dwLastError ||
					TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
					TRUST_E_PROVIDER_UNKNOWN == dwLastError)
				{
					// The file was not signed.
					info = L"The file '" + source_file + L"' is not signed.";
				}
				else
				{
					// The signature was not valid or there was an error 
					// opening the file.
					info = L"An unknown error occurred trying to verify the signature of the '" + source_file + L"' file.";
				}

				break;

			case TRUST_E_EXPLICIT_DISTRUST:
				// The hash that represents the subject or the publisher 
				// is not allowed by the admin or user.
				info = L"The signature is present, but specifically disallowed. Module: " + source_file;
				break;

			case TRUST_E_SUBJECT_NOT_TRUSTED:
				// The user clicked "No" when asked to install and run.
				info = L"The signature is present, but not trusted. Module: " + source_file;
				break;

			case CRYPT_E_SECURITY_SETTINGS:
				/*
				The hash that represents the subject or the publisher
				was not explicitly trusted by the admin and the
				admin policy has disabled user trust. No signature,
				publisher or time stamp errors.
				*/
				info = L"CRYPT_E_SECURITY_SETTINGS - The hash representing the subject or the publisher wasn't explicitly trusted by the admin and admin policy has disabled user trust. No signature, publisher or timestamp errors.";
				break;

			default:
				// The UI was disabled in dwUIChoice or the admin policy 
				// has disabled user trust. lStatus contains the 
				// publisher or time stamp chain error.
				info = source_file + L" check error: " + utils::format_error_wstr(lStatus);
				break;
			}

			// Any hWVTStateData must be released by a call with close.
			WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

			lStatus = WinVerifyTrust(
				NULL,
				&WVTPolicyGUID,
				&WinTrustData);
		}		
		catch (const std::exception& e) {
			info = source_file + L" check error: " + utils::string_to_wide(e.what());
			std::wcerr << info <<  std::endl;
		}
		catch (...) {
			info = source_file + L" check error: " + utils::format_error_wstr(GetLastError());
			std::wcerr << info << std::endl;
		}

		return result;
	}

}
