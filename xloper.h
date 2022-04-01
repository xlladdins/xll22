// xloper.h - XLOPER functions
#pragma once
#include <concepts>
#include <algorithm>
#include <string>
#include "defines.h"
#include "ensure.h"

namespace xll {

#define XLTYPESTR(s) 

	template<class X>
	concept is_xloper = std::is_same_v<X, XLOPER> || std::is_same_v<X, XLOPER12>;

	template<class X> struct traits { };
	template<> struct traits<XLOPER> { 
		using xchar = CHAR; 
		using xint = short int;
		using xref = XLREF;
	};
	template<> struct traits<XLOPER12> { 
		using xchar = XCHAR; 
		using xint = int;
		using xref = XLREF12;
	};

	template<class X>
	struct XREF : public traits<X>::xref
	{
		XREF(unsigned r, unsigned c, unsigned h = 1, unsigned w = 1)
			: traits<X>::xref{r, r + h - 1, c, c + w - 1}
		{ }
	};
	using REF4 = XREF<XLOPER>;
	using REF12 = XREF<XLOPER12>;
	using REF = XREF<XLOPERX>;

	// cyclic indexing
	template<typename T>
	//	requires std::is_integral_v<T>
	inline T xmod(T x, T y)
	{
		if (y == 0)
			return 0;

		T z = x % y;

		return z >= 0 ? z : z + y;
	}

	template<class X>
		requires is_xloper<X>
	inline constexpr int type(const X& x)
	{
		return x.xltype & xlbitmask;
	}

	template<class X>
		requires is_xloper<X>
	inline constexpr bool is_scalar(const X& x)
	{
		return x.xltype & xltypeScalar;
	}

	template<class X>
		requires is_xloper<X>
	inline constexpr int size(const X& x)
	{
		if (xltypeMulti == type(x)) {
			return x.val.array.rows * x.val.array.columns;
		}

		return 1;
	}

	template<class X>
		requires is_xloper<X>
	inline const X& index(const X& x, int i, int j = 0)
	{
		if (xltypeMulti == type(x)) {
			i = xmod(i, x.val.array.rows);
			if (j != 0) {
				j = xmod(j, x.val.array.columns);
			}
			
			return x.val.array.lparray[i + j * x.val.array.columns];
		}

		return x;
	}
	template<class X>
		requires is_xloper<X>
	inline X& index(X& x, int i, int j = 0)
	{
		if (xltypeMulti == type(x)) {
			i = xmod(i, x.val.array.rows);
			if (j != 0) {
				j = xmod(j, x.val.array.columns);
			}

			return x.val.array.lparray[i + j * x.val.array.columns];
		}

		return x;
	}

	template<class X>
		requires is_xloper<X>
	inline const X* begin(const X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.lparray : &x;
	}
	template<class X>
		requires is_xloper<X>
	inline X* begin(X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.lparray : &x;
	}

	template<class X>
		requires is_xloper<X>
	inline const X* end(const X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.lparray + size(x) : &x + 1;
	}
	template<class X>
		requires is_xloper<X>
	inline X* end(X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.lparray[size(x)] : &x + 1;
	}

	template<class X>
		requires is_xloper<X>
	inline X Num(double num)
	{
		return X{ .val = {.num = num}, .xltype = xltypeNum };
	}

	// fixed size string
	template<class X, size_t N>
		requires is_xloper<X>
	class Str : public X {
		using xchar = traits<X>::xchar;
		xchar buf[N];
	public:
		Str(const xchar* s, xchar n = 0)
			: X{ .val = {.str = buf}, .xltype = xltypeStr }
		{
			if (!s) {
				buf[0] = 0;
			}
			else if (n != 0) {
				buf[0] = n;
				for (xchar i = 0; i < n; ++i) {
					buf[i + 1] = s[i];
				}
			}
			else {
				while (*s) {
					++n;
					buf[n] = *s;
					++s;
				}
				buf[0] = n;
			}
		}
	};
#define STR4(s) Str<XLOPER, _countof(s)>(s)
#define STR12(s) Str<XLOPER12, _countof(s)>(s)
#if XLL_VERSION == 12
#define STR(s) STR12(s)
#else
#define STR(s) STR(s)
#endif

	template<class X>
		requires is_xloper<X>
	inline constexpr X Bool(bool xbool)
	{
		return X{ .val = {.xbool = xbool}, .xltype = xltypeBool };
	}
}

inline bool operator==(const XLOPER& x, const XLOPER& y)
{
	if (xll::type(x) != xll::type(y)) {
		return false;
	}

	switch (xll::type(x)) {
	case xltypeNum:
		return x.val.num == y.val.num;
	case xltypeStr:
		if (x.val.str[0] != y.val.str[0]) {
			return false;
		}
		return 0 == strncmp(x.val.str + 1, y.val.str + 1, x.val.str[0]);
	case xltypeBool:
		return x.val.xbool == y.val.xbool;
	case xltypeRef:
		return false;
	case xltypeErr:
		return x.val.err == y.val.err;
	case xltypeMulti:
		if (x.val.array.rows != y.val.array.rows) {
			return false;
		}
		if (x.val.array.columns != y.val.array.columns) {
			return false;
		}
		return std::equal(xll::begin(x), xll::end(x), xll::begin(y));
	case xltypeMissing:
	case xltypeNil:
		return x.xltype == y.xltype;
	case xltypeSRef:
		return false; // x.val.sref.ref == y.val.sref.ref;
	case xltypeInt:
		return x.val.w == y.val.w;
	}

	return false;
}

inline bool operator==(const XLOPER12& x, const XLOPER12& y)
{
	if (xll::type(x) != xll::type(y)) {
		return false;
	}
	switch (xll::type(x)) {
	case xltypeNum:
		return x.val.num == y.val.num;
	case xltypeStr:
		if (x.val.str[0] != y.val.str[0]) {
			return false;
		}
		return 0 == wcsncmp(x.val.str + 1, y.val.str + 1, x.val.str[0]);
	case xltypeBool:
		return x.val.xbool == y.val.xbool;
	case xltypeRef:
		return false;
	case xltypeErr:
		return x.val.err == y.val.err;
	case xltypeMulti:
		if (x.val.array.rows != y.val.array.rows) {
			return false;
		}
		if (x.val.array.columns != y.val.array.columns) {
			return false;
		}
		return std::equal(xll::begin(x), xll::end(x), xll::begin(y));
	case xltypeMissing:
	case xltypeNil:
		return x.xltype == y.xltype;
	case xltypeSRef:
		return false; // x.val.sref.ref == y.val.sref.ref;
	case xltypeInt:
		return x.val.w == y.val.w;
	}

	return false;
}
