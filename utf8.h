// utf8.h - convert UTF-8 to wide character strings
#pragma once
//#include <stringapiset.h>
#include <memory>
#include <WinNls.h>

namespace win {
	// string to malloc'd counted wide character string
	inline wchar_t* mb2wc(const char* s, int n)
	{
		if (!s) {
			return nullptr;
		}

		if (n == 0) {
			wchar_t* ws = (wchar_t*)::malloc(sizeof(wchar_t));
			if (ws) {
				*ws = 0;
			}

			return ws;
		}

		int wn = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s, n, nullptr, 0);
		if (0 == wn) {
			return nullptr;
		}

		wchar_t* ws = (wchar_t*)::malloc(sizeof(wchar_t)*(static_cast<size_t>(wn) + 1));
		if (ws) {
			if (wn != MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s, n, ws + 1, wn)) {
				return nullptr;
			}
			*ws = (wchar_t)wn;
		}

		return ws;
	}
	// counted wide character string to malloc'd counted string
	inline char* wc2mb(const wchar_t* ws, int wn)
	{
		if (!ws) {
			return nullptr;
		}

		if (wn == 0) {
			char* s = (char*)::malloc(sizeof(char));
			if (s) {
				*s = 0;
			}

			return s;
		}

		int n = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws, wn, nullptr, 0, nullptr, nullptr);
		if (0 == n) {
			DWORD err = GetLastError();
			err = ERROR_INSUFFICIENT_BUFFER;
		}

		char* s = (char*)::malloc(static_cast<size_t>(n) + 1);
		if (s) {
			if (n != WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws, wn, s + 1, n, nullptr, nullptr)) {
				return nullptr;
			}
			*s = (char)n;
		}

		return s;
	}

} // namespace win
