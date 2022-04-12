// utf8.h - convert UTF-8 to wide character strings
#pragma once
//#include <stringapiset.h>
#include <memory>
#include <WinNls.h>

namespace win {
	// string to malloc'd counted wide character string
	inline wchar_t* mb2wc(const char* s, int n = 0)
	{
		if (!*s) {
			return nullptr;
		}

		if (0 == n) {
			n = static_cast<int>(strlen(s));
		}

		int wn = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s, n, nullptr, 0);
		if (0 == wn) {
			return nullptr;
		}

		wchar_t* ws = (wchar_t*)::malloc(sizeof(wchar_t)*(wn + 1));
		if (ws) {
			if (wn != MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s, n, ws + 1, wn)) {
				return nullptr;
			}
			*ws = (wchar_t)wn;
		}

		return ws;
	}
	// counted wide character string to malloc'd counted string
	inline char* wc2mb(const wchar_t* ws, int wn = 0)
	{
		if (!*ws) {
			return nullptr;
		}

		if (0 == wn) {
			wn = static_cast<int>(wcslen(ws));
		}

		int n = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws, wn, nullptr, 0, nullptr, nullptr);
		if (0 == n) {
			return nullptr;
		}

		char* s = (char*)::malloc(n + 1);
		if (s) {
			if (n != WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws, wn, s + 1, n, nullptr, nullptr)) {
				return nullptr;
			}
			*s = (char)n;
		}

		return s;
	}

} // namespace win
