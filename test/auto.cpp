// auto.cpp
#include <exception>
#include "xll.h"

#define XLL_ERROR(ex) MessageBoxA(0, ex, "Error", MB_OK)

#pragma region xlAutoOpen
// Called by Excel when the xll is opened.
extern "C" int __declspec(dllexport) WINAPI
xlAutoOpen(void)
{
	try {
		;
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return FALSE;
	}
	catch (...) {
		XLL_ERROR("Unknown exception in xlAutoOpen");

		return FALSE;
	}

	return TRUE;
}
#pragma endregion xlAutoOpen