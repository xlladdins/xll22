// xloper.h
#pragma once
#include <algorithm>
#include <compare>
#include <concepts>
#include <memory>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include "XLCALL.H"
#include "ensure.h"

namespace xll {

	template<class X>
	concept is_xloper = std::is_same_v<X, XLOPER> || std::is_same_v<X, XLOPER12>;

	template<class X> struct traits { };
	template<> struct traits<XLOPER> {
		using type = XLOPER;
		using xchar = CHAR;
		static constexpr xchar xchar_max = std::numeric_limits<xchar>::max();
		using xrw = WORD;
		using xcol = WORD;  // BYTE in REF
		static int Excelv(int xlfn, LPXLOPER operRes, int count, LPXLOPER opers[])
		{
			return ::Excel4v(xlfn, operRes, count, opers);
		}
	};
	template<> struct traits<XLOPER12> {
		using type = XLOPER12;
		using xchar = XCHAR;
		static const xchar xchar_max = 0x7FFF;
		using xrw = RW;
		using xcol = COL;
		static int Excelv(int xlfn, LPXLOPER12 operRes, int count, LPXLOPER12 opers[])
		{
			return ::Excel12v(xlfn, operRes, count, opers);
		}
	};

	template<class X>
	inline int type(const X& x)
	{
		return x.xltype & ~(xlbitDLLFree | xlbitXLFree);
	}
	template<class X>
	inline typename traits<X>::xrw rows(const X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.rows : 1;
	}
	template<class X>
	inline typename traits<X>::xcol columns(const X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.columns : 1;
	}
	template<class X>
	inline auto size(const X& x)
	{
		return rows(x) * columns(x);
	}
	// index
	template<class X>
	inline X* begin(X& x)
	{
		return xltypeMulti == type(x) ? x.val.array.lparray : &x;
	}
	template<class X>
	inline X* end(X& x)
	{
		return xltypeMulti == type(x) ? begin(x) + size(x) : &x + 1;
	}



	// Error types
	template<class X>
		requires is_xloper<X>
	inline constexpr X Err(WORD type)
	{
		return X{ .val = {.err = type}, .xltype = xltypeErr };
	}
	static constexpr XLOPER ErrNA4 = Err<XLOPER>(xlerrNA);
	// ...
	static constexpr XLOPER12 ErrNA = Err<XLOPER12>(xlerrNA);

	template<class X>
	inline constexpr X XMissing{ .val = {.str = nullptr}, .xltype = xltypeMissing };
	inline constexpr XLOPER Missing4 = XMissing<XLOPER>;
	inline constexpr XLOPER12 Missing = XMissing<XLOPER12>;

	template<class X>
	inline constexpr X XNil{ .val = { .str = nullptr }, .xltype = xltypeNil};
	inline constexpr XLOPER Nil4 = XNil<XLOPER>;
	inline constexpr XLOPER12 Nil = XNil<XLOPER12>;

	template<class X>
		requires is_xloper<X>
	class XOPER : public X {
		using xchar = traits<X>::xchar;
		using xrw = traits<X>::xrw;
		using xcol = traits<X>::xcol;
	public:
		using X::xltype;
		using X::val;

		XOPER()
			: X{.xltype = xltypeNil}
		{ }
		XOPER(const X& x)
		{
			if (xltypeStr == ::type(x)) {
				malloc_str(x.val.str[0]);
				std::copy(x.val.str + 1, x.val.str + 1 + x.val.str[0], val.str + 1);
			}
			else if (xltypeMulti == ::type(x)) {
				malloc_multi(x.val.array.rows, x.val.array.columns);
				for (int i = 0; i < size(); ++i) {
					new (val.array.lparray + i)XOPER(x.val.array.lparray[i]);
				}
			}
			else {
				// ensure(is_scalar(x));
				xltype = x.xltype;
				val = x.val;
			}
		}
		explicit XOPER(const XOPER& o)
			: XOPER((X)o)
		{ }
		XOPER& operator=(const XOPER&) = delete;
		~XOPER()
		{
			free_oper();
		}

		bool operator==(const X& x) const
		{
			if (xltype != x.xltype) {
				return false;
			}
			if (xltypeStr == ::type(x)) {
				return equal(x.val.str + 1, x.val.str[0]);
			}
			if (xltypeMulti == ::type(x)) {
				if (rows() != ::rows(x) || columns() != ::columns(x)) {
					return false;
				}
#pragma warning(push)
#pragma warning(disable: 5232) // recursive
				for (int i = 0; i < size(); ++i) {
					if (operator[](i) != x.val.array.lparray[i]) {
						return false;
					}
				}
#pragma warning(pop)

				return true;
			}
			switch (xltype) {
			case xltypeNum:
				return val.num == x.val.num;
			case xltypeBool:
				return val.xbool == x.val.xbool;
			//case xltypeRef:
			case xltypeErr:
				return val.err == x.val.err;
			//case xltypeSRef:
			//	return val.sref.ref == x.val.sref.ref;
			case xltypeInt:
				return val.w == x.val.w;
			}

			return true;
		}
		bool operator==(const XOPER& o) const
		{
			return operator==((X)o);
		}
		int type() const
		{
			return ::type(*this);
		}

		// Num
		explicit XOPER(double num)
			: X{ .val = {.num = num}, .xltype = xltypeNum }
		{ }
		operator double& ()
		{
			ensure(xltypeNum == xltype);
			return val.num;
		}
		operator const double& () const
		{
			ensure(xltypeNum == xltype);
			return val.num;
		}

		// Str
		XOPER(const xchar* str, xchar len)
		{
			malloc_str(len);
			std::copy(str, str + len, val.str + 1);
		}
		explicit XOPER(const xchar* str)
			: XOPER(str, len(str))
		{ }
		template<size_t N>
		XOPER(/*const*/ xchar(&str)[N])
			: XOPER(str, static_cast<xchar>(N - 1))
		{
			static_assert(N <= traits<X>::xchar_max);
		}
		bool operator==(const xchar* str) const
		{
			return equal(str, len(str));
		}
		XOPER& append(const xchar* str, xchar len)
		{
			xchar n = 0;

			if (xltypeNil == xltype) {
				malloc_str(len);
			}
			else if (len) {
				ensure(xltypeStr == xltype);
				n = val.str[0];
				realloc_str(n + len);
			}
			std::copy(str, str + len, val.str + 1 + n);

			return *this;
		}
		XOPER& append(const xchar* str)
		{
			return append(str, len(str));
		}

		explicit XOPER(bool xbool)
			: X{.val = {.xbool = xbool}, .xltype = xltypeBool}
		{ }

		// Multi
		XOPER(xrw r, xcol c)
		{
			malloc_multi(r, c);
			for (int i = 0; i < size(); ++i) {
				new (val.array.lparray + i)XOPER{};
			}
		}
		xrw rows() const noexcept
		{
			return xltypeMulti == type() ? val.array.rows : 1;
		}
		xrw columns() const noexcept
		{
			return xltypeMulti == type() ? val.array.columns : 1;
		}
		auto size() const noexcept
		{
			return rows() * columns();
		}
		const X* array() const
		{
			return xltypeMulti == type() ? val.array.lparray : nullptr;
		}
		XOPER& operator[](int i)
		{
			return val.array.lparray[i];
		}
		const XOPER& operator[](int i) const
		{
			return val.array.lparray[i];
		}
		XOPER& operator()(int i, int j)
		{
			return val.array.lparray[i * columns() + j];
		}
		const XOPER& operator()(int i, int j) const
		{
			return val.array.lparray[i * columns() + j];
		}
		XOPER* begin()
		{
			return xltypeMulti == type() ? /*(XOPER*)*/val.array.lparray : this;
		}
		const XOPER* begin() const
		{
			return xltypeMulti == type() ? val.array.lparray : this;
		}
		XOPER* end()
		{
			return xltypeMulti == type() ? (XOPER*)val.array.lparray + size() : this + 1;
		}
		const XOPER* end() const
		{
			return xltypeMulti == type() ? val.array.lparray + size() : this + 1;
		}
	private:
		static xchar len(const xchar* s)
		{
			size_t n = 0;
			if (s) {
				while (s[n]) {
					++n;
				}
				ensure(n <= traits<X>::xchar_max);
			}

			return static_cast<xchar>(n);
		}
		bool equal(const xchar* str, int len) const
		{
			if (val.str[0] != len) {
				return false;
			}

			return std::equal(str, str + len, val.str + 1);
		}

		void free_oper()
		{
			if (xltypeStr == xltype) {
				free_str();
			}
			else if (xltypeMulti == xltype) {
				free_multi();
			}
			else {
				xltype = xltypeNil;
			}
		}
		// allocate and set str[0]
		void malloc_str(xchar len)
		{
			ensure(len <= traits<X>::xchar_max);
			val.str = (xchar*)::malloc((len + 1) * sizeof(xchar));
			ensure(val.str);
			val.str[0] = len;
			xltype = xltypeStr;
		}
		void realloc_str(xchar len)
		{
			ensure(xltypeStr == xltype);
			if (val.str[0] != len) {
				ensure(len <= traits<X>::xchar_max);
				val.str = (xchar*)::realloc(val.str, (len + 1) * sizeof(xchar));
				ensure(val.str);
				val.str[0] = len;
			}
		}
		void free_str()
		{
			if (xltypeStr == xltype) {
				::free(val.str);
			}
			else if ((xltypeStr | xlbitXLFree) == xltype) {
				X* x[1] = { (X*)this };
				traits<X>::Excelv(xlFree, 0, 1, x);
			}
			xltype = xltypeNil;
		}
		void malloc_multi(xrw r, xcol c)
		{
			val.array.rows = r;
			val.array.columns = c;
			if (size()) {
				val.array.lparray = (X*)malloc(r * c * sizeof(X));
				ensure(val.array.lparray);
			}
			else {
				val.array.lparray = nullptr;
			}
			xltype = xltypeMulti;
		}
		void realloc_multi(xrw r, xcol c)
		{
			ensure(xltypeMulti == xltype);
			if (size() != r * c) {
				val.array.lparray = (X*)::realloc(val.array.lparray, r * c * sizeof(X));
				ensure(val.array.lparray);
			}
			val.array.rows = r;
			val.array.columns = c;
		}
		void free_multi()
		{
			if (xltypeMulti == xltype) {
				if (size()) {
					::free(val.array.lparray);
				}
				xltype = xltypeNil;
			}
		}
	};
	using OPER4 = XOPER<XLOPER>;
	using OPER = XOPER<XLOPER12>;
/*
	inline const XLOPER True4 = XOPER<XLOPER>(true);
	inline XLOPER False4 = XOPER<XLOPER>(false);
	inline XLOPER12 True = XOPER<XLOPER12>(true);
	inline XLOPER12 False = XOPER<XLOPER12>(false);
*/
}
