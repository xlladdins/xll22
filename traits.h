// traits.h - XLOPER traits
#pragma once
#include "defines.h"
#include "utf8.h"

namespace xll {

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
