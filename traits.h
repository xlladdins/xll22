// traits.h - XLOPER traits
#pragma once

#include <concepts>
#include <type_traits>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include "XLCALL.H"
#include "utf8.h"

namespace xll {

	// full name of dll set in DllMain
	inline const TCHAR* module_text = nullptr;

	template<class X>
	concept is_xloper
		= std::is_same_v<XLOPER, X> || std::is_same_v<XLOPER12, X>;
	template<class X>
	concept is_base_of_xloper
		= std::is_base_of_v<XLOPER, X> || std::is_base_of_v<XLOPER12, X>;
	template<class X, class Y>
	concept both_base_of_xloper
		= (std::is_base_of_v<XLOPER, X> && std::is_base_of_v<XLOPER, Y>)
		|| (std::is_base_of_v<XLOPER12, X> && std::is_base_of_v<XLOPER12, Y>);

	template<class X> struct traits { };

	template<> struct traits<XLOPER> {
		using type = XLOPER;
		using typex = XLOPER12;
		using xchar = CHAR;
		using charx = XCHAR;
		static constexpr xchar xchar_max = 0xFF;
		using xrw = WORD;
		using xcol = BYTE;
		// xrw_max, xcol_max, xdim?
		using xref = XLREF;
		static xchar* cvt(const charx* str, int len)
		{
			return win::wc2mb(str, len);
		}
		static int Excelv(int xlfn, LPXLOPER operRes, int count, LPXLOPER opers[])
		{
			return ::Excel4v(xlfn, operRes, count, opers);
		}
	};

	template<> struct traits<XLOPER12> {
		using type = XLOPER12;
		using typex = XLOPER;
		using xchar = XCHAR;
		using charx = CHAR;
		static const xchar xchar_max = 0x7FFF;
		using xrw = RW;
		using xcol = COL;
		// xrw_max = 2^20
		// xcol_max
		using xref = XLREF12;
		static xchar* cvt(const charx* str, int len)
		{
			return win::mb2wc(str, len);
		}
		static int Excelv(int xlfn, LPXLOPER12 operRes, int count, LPXLOPER12 opers[])
		{
			return ::Excel12v(xlfn, operRes, count, opers);
		}
	};

} // namespace xll
