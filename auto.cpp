// auto.cpp
#include <exception>
#include "xll.h"

using namespace xll;

#define XLL_ERROR(msg) MessageBoxA(0, msg, "Error", MB_OK)
#define XLL_WARN(msg)  MessageBoxA(0, msg, "Warning", MB_OK)
#define XLL_INFO(msg)  MessageBoxA(0, msg, "Information", MB_OK)

#pragma region xlAutoOpen
// Called by Excel when the xll is opened.
extern "C" int __declspec(dllexport) WINAPI
xlAutoOpen(void)
{
	try {
		xll::XArgs<XLOPER12> xai_foo(XMacro<XLOPER12>("foo", "FOO").Category("Bar"));
		//macro.Category("Bar");
		//xll::Register(macro);
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