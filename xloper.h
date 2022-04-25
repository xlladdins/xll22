// xloper.h
#pragma once
#include <algorithm>
#include <compare>
#include <concepts>
#include <memory>
#include "xlref.h"
//#include "utf8.h"

namespace xll {

	// turn off memory management bits
	template<class X>
	inline auto type(const X& x) noexcept
	{
		return x.xltype & ~(xlbitDLLFree | xlbitXLFree);
	}

	template<class X>
	inline auto rows(const X& x)
	{
		return type(x) == xltypeMulti ? x.val.array.rows : 1;
	}
	template<class X>
	inline auto columns(const X& x)
	{
		return type(x) == xltypeMulti ? x.val.array.columns : 1;
	}
	template<class X>
	inline auto size(const X& x)
	{
		return rows(x) * columns(x);
	}

	// 1-d index
	template<class X>
		requires is_base_of_xloper<X>
	inline X& index(X& x, int i)
	{
		return static_cast<X&>(x.val.array.lparray[i]);
	}
	template<class X>
		requires is_base_of_xloper<X>
	inline const X& index(const X& x, int i)
	{
		return static_cast<const X&>(x.val.array.lparray[i]);
	}

	// 2-d index
	template<class X>
		requires is_base_of_xloper<X>
	inline X& index(X& x, int i, int j)
	{
		return static_cast<X&>(x.val.array.lparray[i * columns(x) + j]);
	}
	template<class X>
		requires is_base_of_xloper<X>
	inline const X& index(const X& x, int i, int j)
	{
		return static_cast<const X&>(x.val.array.lparray[i * columns(x) + j]);
	}

	// STL friendly
	template<class X>
		requires is_base_of_xloper<X>
	inline X* begin(X& x)
	{
		return static_cast<X*>(type(x) == xltypeMulti ? x.val.array.lparray : &x);
	}
	template<class X>
		requires is_base_of_xloper<X>
	inline const X* begin(const X& x)
	{
		return static_cast<const X*>(type(x) == xltypeMulti ? x.val.array.lparray : &x);
	}
	template<class X>
		requires is_base_of_xloper<X>
	inline X* end(X& x)
	{
		return begin(x) + size(x);
	}
	template<class X>
		requires is_base_of_xloper<X>
	inline const X* end(const X& x)
	{
		return begin(x) + size(x);
	}

	template<class X, class Y>
		requires both_base_of_xloper<X,Y>
	inline bool equal(const X& x, const Y& y)
	{
		if (type(x) != type(y)) {
			return false;
		}

		switch (type(x)) {
		case xltypeNum:
			return x.val.num == y.val.num;
		case xltypeStr:
			if (x.val.str[0] != y.val.str[0]) {
				return false;
			}
			return std::equal(x.val.str + 1, x.val.str + 1 + x.val.str[0], y.val.str + 1);
		case xltypeBool:
			return x.val.xbool == y.val.xbool;
		case xltypeRef: {
			if (x.val.mref.idSheet != y.val.mref.idSheet) {
				return false;
			}

			const auto& xm = *x.val.mref.lpmref;
			const auto& ym = *y.val.mref.lpmref;

			return std::equal(begin(xm), end(xm), begin(ym), end(ym));
		}
		case xltypeErr:
			return x.val.err == y.val.err;
		case xltypeMulti:
			if (rows(x) != rows(y) || columns(x) != columns(y)) {
				return false;
			}
			return std::equal(begin(x), end(x), begin(y), end(y), equal<X,Y>);
		case xltypeSRef:
			return x.val.sref.ref == y.val.sref.ref;
		case xltypeInt:
			return x.val.w == y.val.w;
		case xltypeBigData:
			if (x.val.bigdata.cbData != y.val.bigdata.cbData) {
				return false;
			}
			return 0 == memcmp(x.val.bigdata.h.lpbData, y.val.bigdata.h.lpbData, x.val.bigdata.cbData);
		}

		return true;
	}



#pragma region XOPER
	// value type for XLOPER
	template<class X>
		requires is_xloper<X>
	class XOPER : public X {
		using xchar = traits<X>::xchar;
		using charx = traits<X>::charx;
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
				malloc_multi(::rows(x), ::columns(x));
				for (int i = 0; i < size(); ++i) {
					new (val.array.lparray + i)XOPER(x.val.array.lparray[i]);
				}
			}
			else {
				// ensure(is_scalar(x));
				xltype = ::type(x);
				val = x.val;
			}
		}
		explicit XOPER(const XOPER& o)
			: XOPER((X)o)
		{ }
		XOPER(XOPER&& o) noexcept
		{
			val = o.val;
			xltype = o.xltype;
			o.xltype = xltypeNil;
		}
		XOPER& operator=(const X& x)
		{
			return *this = XOPER(x);
		}
		XOPER& operator=(XOPER x)
		{
			swap(x);

			return *this;
		}
		~XOPER()
		{
			free_oper();
		}

		[[nodiscard]] int type() const
		{
			return xll::type(*this);
		}

		void swap(XOPER& x) noexcept
		{
			using std::swap;

			swap(xltype, x.xltype);
			swap(val, x.val);
		}

		bool operator==(const X& x) const noexcept
		{
			return xll::equal(*this, x);
		}
		bool operator==(const XOPER& o) const noexcept
		{
			return operator==((X)o);
		}

#pragma region Num
		explicit XOPER(double num)
			: X{ .val = {.num = num}, .xltype = xltypeNum }
		{ }
		XOPER& operator=(double num)
		{
			return *this = XOPER(num);
		}
		friend bool operator==(const X& x, double num) noexcept
		{
			return xltypeNum == ::type(x) and x.val.num == num;
		}
		operator double () const
		{
			switch (type()) {
			case xltypeNum:
				return val.num;
			case xltypeBool:
				return val.xbool;
			case xltypeInt:
				return val.w;
			}

			return std::numeric_limits<double>::quiet_NaN();
		}
#pragma endregion Num

#pragma region Str
		XOPER(const xchar* str, xchar len)
		{
			malloc_str(len);
			std::copy(str, str + len, val.str + 1);
		}
		XOPER(const charx* str, charx len)
		{
			xltype = xltypeStr;
			val.str = traits<X>::cvt(str, len);
		}
		explicit XOPER(const xchar* str)
			: XOPER(str, len(str))
		{ }
		explicit XOPER(const charx* str)
			: XOPER(str, len(str))
		{ }
		template<size_t N>
		XOPER(const xchar(&str)[N])
			: XOPER(str, static_cast<xchar>(N - 1))
		{
			static_assert(N <= traits<X>::xchar_max);
		}
		template<size_t N>
		XOPER(const charx(&str)[N])
			: XOPER(str, static_cast<xchar>(N - 1))
		{
			static_assert(N <= traits<X>::xchar_max);
		}
		XOPER& operator=(const xchar* str)
		{
			return *this = XOPER(str);
		}
		XOPER& operator=(const charx* str)
		{
			return *this = XOPER(str);
		}
		bool operator==(const xchar* str) const noexcept
		{
			return equal(str, len(str));
		}
		bool operator==(const charx* str) const noexcept
		{
			return equal(str, len(str));
		}
		XOPER& append(const xchar* str, xchar len)
		{
			xchar n = 0;

			if (xltypeNil == xltype) {
				return *this = XOPER(str, len);
			}
			if (xltypeStr != xltype) {
				return *this = XErr<X>(xlerrValue);
			}
			if (len) {
				n = val.str[0];
				realloc_str(n + len);
				std::copy(str, str + len, val.str + 1 + n);
			}

			return *this;
		}
		XOPER& append(const xchar* str)
		{
			return append(str, len(str));
		}
#pragma endregion Str

#pragma region Bool
		explicit XOPER(bool b)
			: X{.val = {.xbool = b}, .xltype = xltypeBool}
		{ }
		///*
		friend bool operator==(const X& x, bool b) noexcept
		{
			return xltypeBool == ::type(x) and static_cast<bool>(x.val.xbool) == b;
		}
		//*/
#pragma endregion Bool

		// Ref

#pragma region Err
		explicit XOPER(enum Err err)
			: X{ .val = {.err = err}, .xltype = xltypeErr }
		{ }
#pragma endregion Err

#pragma region Multi
		XOPER(xrw r, xcol c)
		{
			malloc_multi(r, c);
		}
		XOPER& resize(xrw r, xcol c)
		{
			if (0 == r * c) {
				free_oper();
			}
			else if (xltypeMulti == xltype) {
				realloc_multi(r, c); // fix up if old col != c???
			}
			else {
				XOPER<X> o = std::move(*this);
				malloc_multi(r, c);
				operator[](0) = std::move(o);
			}

			return *this;
		}
		// stack vertically if same number of columns
		XOPER& stack(const X& x)
		{
			if (xltypeNil == type()) {
				return *this = x;
			}

			if (overlap(x)) {
				return stack(XOPER<X>(x));
			}

			if (columns() != ::columns(x)) {
				return operator=(XErr<X>(xlerrValue));
			}

			if (xltypeMulti != type()) {
				auto o0{ *this };
				free_oper();
				malloc_multi(1, 1);
				operator[](0) = o0;
			}
			auto r = rows();
			resize(r + ::rows(x), columns());
			std::copy(::begin(x), ::end(x), begin() + static_cast<size_t>(r) * columns());

			return *this;
		}
		xrw rows() const noexcept
		{
			return ::rows(*this);
		}
		xcol columns() const noexcept
		{
			return ::columns(*this);
		}
		auto size() const noexcept
		{
			return ::size(*this);
		}
		const X* array() const
		{
			return xltypeMulti == type() ? val.array.lparray : nullptr;
		}
		XOPER& operator[](int i)
		{
			return ::index(*this, i);
		}
		const XOPER& operator[](int i) const
		{
			return ::index(*this, i);
		}
		XOPER& operator()(int i, int j)
		{
			return ::index(*this, i, j);
		}
		const XOPER& operator()(int i, int j) const
		{
			return ::index(*this, i, j);
		}
#pragma endregion Multi

		XOPER* begin()
		{
			return ::begin(*this);
		}
		const XOPER* begin() const
		{
			return ::begin(*this);
		}
		XOPER* end()
		{
			return ::end(*this);
		}
		const XOPER* end() const
		{
			return ::end(*this);
		}
	private:
		template<class T>
		static T len(const T* s)
		{
			T n = 0;
			if (s) {
				while (s[n]) {
					++n;
				}
			}

			return n;
		}
		template<class T>
		bool equal(const T* str, int len) const noexcept
		{
			if (val.str[0] != len) {
				return false;
			}

			return std::equal(str, str + len, val.str + 1);
		}
		// true if memory overlaps with x
		bool overlap(const X& x) const
		{
			return (begin() <= ::begin(x) and ::begin(x) < end())
				or (begin() < ::end(x) and ::end(x) <= end());
		}

		void free_oper()
		{
			if (xlbitXLFree & xltype) {
				X* x[1] = { (X*)this };
				traits<X>::Excelv(xlFree, 0, 1, x);
				xltype = xltypeNil;
			}
			else if (xltypeStr == xltype) {
				free_str();
			}
			else if (xltypeMulti == xltype) {
				free_multi();
			}
		}

		// allocate and set str[0]
		void malloc_str(xchar len)
		{
			val.str = (xchar*)::malloc((static_cast<size_t>(len) + 1) * sizeof(xchar));
			if (val.str) {
				val.str[0] = len;
				xltype = xltypeStr;
			}
			else {
				val.err = xlerrValue;
				xltype = xltypeErr;
			}
		}
		void realloc_str(xchar len)
		{
			if (xltypeStr != xltype) {
				val.err = xlerrValue;
				xltype = xltypeErr;

				return;
			}

			if (val.str[0] != len) {
				val.str = (xchar*)::realloc(val.str, (static_cast<size_t>(len) + 1) * sizeof(xchar));
				if (val.str) {
					val.str[0] = len;
				}
				else {
					val.err = xlerrValue;
					xltype = xltypeErr;
				}
			}
		}
		void free_str()
		{
			if (xltypeStr != xltype) {
				val.err = xlerrValue;
				xltype = xltypeErr;
			}
			else {
				::free(val.str);
				xltype = xltypeNil;
			}
		}

		void malloc_multi(xrw r, xcol c)
		{
			size_t n = r * c;
			if (n) {
				val.array.lparray = (X*)malloc(n * sizeof(X));
				if (val.array.lparray) {
					for (int i = 0; i < n; ++i) {
						new (val.array.lparray + i) XOPER{};
					}
					val.array.rows = r;
					val.array.columns = c;
					xltype = xltypeMulti;
				}
				else {
					val.err = xlerrValue;
					xltype = xltypeErr;
				}
			}
			else {
				xltype = xltypeNil;
			}
		}
		void realloc_multi(xrw r, xcol c)
		{
			if (xltypeMulti != xltype) {
				// free_oper(); // ???
				val.err = xlerrValue;
				xltype = xltypeErr;

				return;
			}

			if (0 == r * c) {
				free_multi();

				return;
			}

			auto n = size(); // old size
			val.array.rows = r;
			val.array.columns = c;

			if (n > size()) {
				std::for_each(end(), begin() + n, [](auto& o) { o.free_oper(); });
			}
			else if (n < size()) {
				val.array.lparray = (X*)::realloc(val.array.lparray, size() * sizeof(X));
				if (val.array.lparray) {
					for (int i = n; i < size(); ++i) {
						new (val.array.lparray + i) XOPER{};
					}
				}
				else {
					val.err = xlerrValue;
					xltype = xltypeErr;
				}
			}
		}
		void free_multi()
		{
			if (xltypeMulti == xltype) {
				std::for_each(begin(), end(), [](auto& o) { o.free_oper(); });
				::free(val.array.lparray);
				xltype = xltypeNil;
			}
			else {
				val.err = xlerrValue;
				xltype = xltypeErr;
			}
		}
	};
#pragma endregion XOPER
	using OPER4 = XOPER<XLOPER>;
	using OPER = XOPER<XLOPER12>;

	template<> struct traits<XOPER<XLOPER>> {
		using type = XLOPER;
		using xref = XLREF;
	};
	template<> struct traits<XOPER<XLOPER12>> {
		using type = XLOPER12;
		using xref = XLREF12;
	};
	/*
	inline const XLOPER True4 = XOPER<XLOPER>(true);
	inline XLOPER False4 = XOPER<XLOPER>(false);
	inline XLOPER12 True = XOPER<XLOPER12>(true);
	inline XLOPER12 False = XOPER<XLOPER12>(false);
*/
}
