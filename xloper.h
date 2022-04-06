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
		using xcol = BYTE;
		static int Excelv(int xlfn, LPXLOPER operRes, int count, LPXLOPER opers[])
		{
			ensure(xlretSuccess == ::Excel4v(xlfn, operRes, count, opers));
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
			ensure(xlretSuccess == ::Excel12v(xlfn, operRes, count, opers));
		}
	};

	// full name of dll set in DllMain
	inline const TCHAR* module_text = nullptr;

	// floating point double
	template<class X = XLOPER12>
		requires is_xloper<X>
	struct XNum : public X {
		XNum(double num)
			: X{ .val = {.num = num}, .xltype = xltypeNum }
		{ }
	};
	using Num4 = XNum<XLOPER>;
	using Num = XNum<XLOPER12>;

	// string
	template<class X>
		requires is_xloper<X>
	class XStr : public virtual X {
		using X::xltype;
		using X::val;
		using xchar = traits<X>::xchar;

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

		// allocate and set str[0]
		void alloc(xchar len)
		{
			ensure(len <= traits<X>::xchar_max);
			val.str = (xchar*)::malloc((len + 1) * sizeof(xchar));
			ensure(val.str);
			val.str[0] = len;
			xltype = xltypeStr;
		}
		void realloc(xchar len)
		{
			ensure(xltypeStr == xltype);
			if (val.str[0] != len) {
				ensure(len <= traits<X>::xchar_max);
				val.str = (xchar*)::realloc(val.str, (len + 1) * sizeof(xchar));
				ensure(val.str);
				val.str[0] = len;
			}
		}
		void dealloc()
		{
			ensure(xltypeStr == xltype);
			::free(val.str);
			xltype = xltypeNil;
		}
		// copy to allocated same size string
		void copy(const xchar* str, xchar len)
		{
			ensure(xltypeStr == xltype);
			ensure(val.str[0] == len);
			std::copy(str, str + len, val.str + 1);
		}
	public:
		XStr()
			: XStr(nullptr, 0)
		{ }
		XStr(const xchar* str, xchar len)
		{
			alloc(len);
			copy(str, len);
		}
		template<size_t N>
		XStr(const xchar (&str)[N])
			: XStr(str, static_cast<xchar>(N - 1))
		{ }
		explicit XStr(const xchar* str)
			: XStr(str, len(str))
		{ }
		explicit XStr(const XStr& str)
			: XStr(str.val.str + 1, str.val.str[0])
		{
		}
		XStr& operator=(const XStr& str)
		{
			realloc(str.val.str[0]);
			copy(str.val.str + 1, str.val.str[0]);

			return *this;
		}
		virtual ~XStr()
		{
			dealloc();
		}
		bool equal(const xchar* str, int len) const
		{
			if (val.str[0] != len) {
				return false;
			}

			return std::equal(str, str + len, val.str + 1);
		}
		bool operator==(const xchar* str) const
		{
			return equal(str, len(str));
		}
		bool operator==(const XStr& str) const
		{
			return equal(str.val.str + 1, str.val.str[0]);
		}
		xchar& operator[](xchar i)
		{
			ensure(i < val.str[0]);
			return val.str[i + 1];
		}
		xchar operator[](xchar i) const
		{
			ensure(i < val.str[0]);
			return val.str[i + 1];
		}
		XStr& append(const xchar* str, xchar len)
		{
			if (len) {
				xchar n = val.str[0];
				realloc(n + len);
				std::copy(str, str + len, val.str + 1 + n);
			}

			return *this;
		}
		XStr& append(const xchar* str)
		{
			return append(str, len(str));
		}
	};
	using Str4 = XStr<XLOPER>;
	using Str = XStr<XLOPER12>;

	// Booliean value
	template<class X = XLOPER12>
		requires is_xloper<X>
	inline constexpr X Bool(bool xbool)
	{
		return X{ .val = {.xbool = xbool}, .xltype = xltypeBool };
	}

	static constexpr XLOPER True4 = Bool<XLOPER>(true);
	static constexpr XLOPER False4 = Bool<XLOPER>(false);
	static constexpr XLOPER12 True = Bool<XLOPER12>(true);
	static constexpr XLOPER12 False = Bool<XLOPER12>(false);

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
	class XMulti : public virtual X {
		using X::xltype;
		using X::val;
		using xrw = traits<X>::xrw;
		using xcol = traits<X>::xcol;
		void alloc(xrw r, xcol c)
		{
			val.array.rows = r;
			val.array.columns = c;
			val.array.lparray = (X*)malloc(r * c * sizeof(X));
		}
		void realloc(xrw r, xcol c)
		{
			if (size() != r * c) {
				val.array.lparray = (X*)::realloc(val.array.lparray, r * c * sizeof(X));
			}
			val.array.rows = r;
			val.array.columns = c;
		}
		void dealloc()
		{
			if (size()) {
				::free(val.array.lparray);
			}
			val.array.rows = 0;
			val.array.columns = 0;
			val.array.lparray = nullptr;
		}
		void fill(const X& x)
		{
			std::copy(begin(), end(), x);
		}
	public:
		XMulti() noexcept
			: X{ .val = {.array = {.lparray = nullptr, .rows = 0, .columns = 0}} }
		{ }
		XMulti(xrw r, xcol c)
			: X{ .val = {.array = {.lparray = nullptr, .rows = r, .columns = c}} }
		{
			alloc(r, c);
			fill(XNil<X>);
		}
		virtual ~XMulti()
		{
			dealloc();
		}
		XMulti& resize(int r, int c)
		{
			realloc(r, c);

			return *this;
		}
		xrw rows() const noexcept
		{
			return val.array.rows;
		}
		xrw columns() const noexcept
		{
			return val.array.columns;
		}
		auto size() const noexcept
		{
			return rows() * columns();
		}
		X& operator[](int i)
		{
			return val.array.lparray[i];
		}
		const X& operator[](int i) const
		{
			return val.array.lparray[i];
		}
		X& operator()(int i, int j)
		{
			return val.array.lparray[i * columns() + j];
		}
		const X& operator()(int i, int j) const
		{
			return val.array.lparray[i * columns() + j];
		}
		X* begin()
		{
			return val.array.lparray;
		}
		const X* begin() const
		{
			return val.array.lparray;
		}
		X* end()
		{
			return val.array.lparray + size();
		}
		const X* end() const
		{
			return val.array.lparray + size();
		}
	};
	using Multi4 = XMulti<XLOPER>;
	using Multi = XMulti<XLOPER12>;

	template<class X>
	inline constexpr X XMissing{ .val = {.num = 0}, .xltype = xltypeMissing };
	inline constexpr XLOPER Missing4 = XMissing<XLOPER>;
	inline constexpr XLOPER12 Missing = XMissing<XLOPER12>;

	template<class X>
	inline constexpr X XNil{ .val = { .num = 0 }, .xltype = xltypeNil};
	inline constexpr XLOPER Nil4 = XNil<XLOPER>;
	inline constexpr XLOPER12 Nil = XNil<XLOPER12>;

	template<class X>
		requires is_xloper<X>
	class XOPER : public XStr<X>, XMulti<X> {
		using X::xltype;
		using X::val;
		using xchar = traits<X>::xchar;
		using xrw = traits<X>::xrw;
		using xcol = traits<X>::xcol;
	public:
		XOPER()
			: X(XNil<X>)
		{ }
		XOPER(const X& x)
		{
			if (xltypeStr == x.xltype) {
				XStr<X>(x.val.str + 1, x.val.str[0]);
			}
			else if (xltypeMulti == x.xltype) {
				XMulti<X>(x.val.array.rows, x.val.array.columns);
				std::copy(x.val.array.lparray, x.val.lparray + size(), val.array.lparray);
			}
			else {
				xltype = x.xltype;
				val = x.val;
			}
		}
		XOPER& operator=(const XOPER&) = delete;
		~XOPER() noexcept
		{ 
			if (xltype & xlbitXLFree) {
				X* x[1] = { (X*)this };
				traits<X>::Excelv(xlFree, 0, 1, x);
			}
		}
		int type() const
		{
			return type(*this);
		}
		auto size() const
		{
			return XMulti<X>::size();
		}
		explicit XOPER(double num)
			: X(XNum<X>(num))
		{ }
		explicit XOPER(const xchar* str)
			: XStr<X>(str)
		{ }
		XOPER(xrw r, xcol c)
			: XMulti<X>(r, c)
		{ }

		XOPER& operator[](int i)
		{
			if (type() == xltypeMulti) {
				return val.array.lparray[i];
			}
			ensure(i == 0);
			return *this;
		}
		const XOPER& operator[](int i) const
		{
			if (type() == xltypeMulti) {
				return val.array.lparray[i];
			}
			ensure(i == 0);
			return *this;
		}
	};
	using OPER4 = XOPER<XLOPER>;
	using OPER = XOPER<XLOPER12>;
}
