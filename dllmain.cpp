// dllmain.cpp : Defines the entry point for the DLL application.
#include "xll.h"

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  reason, LPVOID /*lpReserved*/)
{
    static TCHAR buf[MAX_PATH + 1];
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        buf[0] = static_cast<TCHAR>(GetModuleFileName(hModule, buf + 1, MAX_PATH));
        xll::module_text = buf; // counted string
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

