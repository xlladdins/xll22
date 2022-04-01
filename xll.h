// xll.h - Excel add-in header
#pragma once
#include <concepts>
#include <memory>
#include "xloper.h"

namespace xll {

	// null terminated to counted
	template<class T>
	inline constexpr T* c2p(T* s)
	{
		if (s && *s) {
			T n = 0;
			for (T* t = s; t && *t; ++t) {
				++n;
			}
			for (T m = n; m != 0; --m) {
				s[m] = s[m - 1];
			}
			s[0] = n;
		}

		return s;
	}
	// counted to null terminated
	template<class T>
	inline constexpr T* p2c(T* s)
	{
		if (s && *s) {
			T n = s[0];
			for (T i = 0; i < n; ++i) {
				s[i] = s[i + 1];
			}
			s[n] = 0;
		}

		return s;
	}

	// counted string to new[]d counted wide character string
	inline wchar_t* mb2wc(const char* s)
	{
		if (!*s) {
			return nullptr;
		}

		int wn = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s + 1, s[0], nullptr, 0);
		ensure(0 != wn);

		wchar_t* ws = new wchar_t[wn + 1];
		if (ws) {
			ensure(wn == MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, s + 1, s[0], ws + 1, wn));
			*ws = (wchar_t)wn;
		}

		return ws;
	}
	// counted wide character string to new[]d counted string
	inline char* wc2mb(const wchar_t* ws)
	{
		if (!*ws) {
			return nullptr;
		}

		int n = WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws + 1, ws[0], nullptr, 0, nullptr, nullptr);
		ensure(0 != n);

		char* s = new char[n + 1];
		if (s) {
			ensure(n == WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS, ws + 1, ws[0], s + 1, n, nullptr, nullptr));
			*s = (char)n;
		}

		return s;
	}



	template<class X>
		requires is_xloper<X>
	class XOPER final : public X {
		using xchar = traits<X>::xchar;
		void alloc_str(const xchar* str, xchar len = 0)
		{
			static xchar zero = 0;
			xltype = xltypeStr;
			val.str = &zero;
			if constexpr (std::is_same_v<X, XLOPER>) {
				auto n = strlen(str);
				ensure(n <= 0xFF);
				val.str = new xchar[n + 1];
				val.str[0] = (xchar)n;
				strncpy_s(val.str + 1, n, str, n);
			}
			if constexpr (std::is_same_v<X, XLOPER12>) {
				auto n = wcslen(str);
				ensure(n <= 0x7FFF);
				val.str = new xchar[n + 1];
				val.str[0] = (xchar)n;
				wcsncpy_s(val.str + 1, n, str, n);
			}
		}
		void dealloc_str()
		{
			ensure(type() == xltypeStr);
			if (xltypeStr == xltype) { // not type()

				if (val.str && val.str[0]) {
					delete[] val.str;
				}
			}
			if (xltype & xlbitXLFree) {
				typename traits<X>::type* x[1] = {this};
				traits<X>::Excelv(xlFree, 0, 1, x);
			}
			xltype = xltypeNil;
		}
		void allocate(const X& x)
		{
			switch (type()) {
			case xltypeStr:
				alloc_str(x.val.str + 1, x.val.str[0]);
				break;
			case xltypeMulti:
				break;
			case xltypeBigData:
				break;
			default:
				xltype = x.xltype;
				val = x.val;
			}
		}
		void deallocate() noexcept
		{
			switch (type()) {
			case xltypeStr:
				dealloc_str();
				break;
			case xltypeMulti:
				break;
			case xltypeBigData:
				break;
			}
			xltype = xltypeNil;
		}
	public:
		using X::xltype;
		using X::val;

		// default to Nil
		XOPER()
			: X{ XNil<X>{} }
		{ }
		XOPER(const X& x)
		{
			allocate(x);
		}
		XOPER(const XOPER& o)
		{
			allocate(o);
		}
		XOPER(XOPER&& o) noexcept
			: val(o.val), xltype(o.xltype)
		{
			o.xltype = xltypeNil;
		}
		XOPER& operator=(XOPER o) noexcept
		{
			std::swap(xltype, o.xltype);
			std::swap(val, o.val);

			return *this;
		}
		~XOPER() noexcept
		{
			deallocate();
		}

		int type() const
		{
			return xll::type<X>(*this);
		}

		bool is_scalar() const
		{
			return type() & xltypeScalar;
		}

		bool operator==(const X& x) const
		{
			return ::operator==(*this, x);
		}
		bool operator!=(const X& x) const
		{
			return !(operator==(x));
		}

		// xltypeNum
		explicit XOPER(double num)
			: X{Num<X>(num)}
		{ }
		XOPER& operator=(double num)
		{
			deallocate();
			xltype = xltypeNum;
			val.num = num;

			return *this;
		}
		operator double() const
		{
			switch (type()) {
			case xltypeNum:
				return val.num;
			case xltypeBool:
				return val.xbool;
			case xltypeInt:
				return val.w;
			}

			return std::numeric_limits<double>::quite_NaN();
		}
		operator double&()
		{
			ensure(xltypeNum == type());

			return val.num;
		}

		// xltypeStr
		XOPER(const xchar* str, xchar len)
		{
			alloc_str(str, len);
		}
		explicit XOPER(const xchar* str)
		{
			alloc_str(str);
		}
		/*
		explicit XOPER(const char* str)
		{

		}
		*/
		XOPER& operator=(const xchar* str)
		{
			deallocate();
			alloc_str(str);

			return *this;
		}
	};
	using OPER4 = XOPER<XLOPER>;
	using OPER12 = XOPER<XLOPER12>;
	using OPERX = XOPER<XLOPERX>;
}