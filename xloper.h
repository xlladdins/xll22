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
	concept is_xloper = std::is_same_v<XLOPER, X> || std::is_same_v<XLOPER12, X>;
	template<class X>
	concept is_base_of_xloper = std::is_base_of_v<XLOPER, X> || std::is_base_of_v<XLOPER12, X>;

	template<class X> struct traits { };
	template<> struct traits<XLOPER> {
		using type = XLOPER;
		using xchar = CHAR;
		static constexpr xchar xchar_max = std::numeric_limits<xchar>::max();
		using xrw = WORD;
		using xcol = WORD;  // BYTE in REF
		// xrw_max, xcol_max
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
		// xrw_max, xcol_max
		static int Excelv(int xlfn, LPXLOPER12 operRes, int count, LPXLOPER12 opers[])
		{
			return ::Excel12v(xlfn, operRes, count, opers);
		}
	};

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
		requires is_base_of_xloper<X> && is_base_of_xloper<Y>
	inline bool equal(const X& x, const Y& y)
	{
		if (type(x) != type(y)) {
			return false;
		}
		if (xltypeNum == type(x)) {
			return x.val.num == y.val.num;
		}
		if (xltypeStr == type(x)) {
			if (x.val.str[0] != y.val.str[0]) {
				return false;
			}
			return std::equal(x.val.str + 1, x.val.str + 1 + x.val.str[0], y.val.str + 1);
		}
		if (xltypeBool == type(x)) {
			return x.val.xbool == y.val.xbool;
		}
		// if (xltypeRef == type(x)) { ... }
		if (xltypeErr == type(x)) {
			return x.val.err == y.val.err;
		}
		if (xltypeMulti == type(x)) {
			if (::rows(x) != ::rows(y) || columns(x) != ::columns(y)) {
				return false;
			}
#pragma warning(push)
#pragma warning(disable: 5232) // recursive
			for (int i = 0; i < size(x); ++i) {
				if (!equal(index(x, i), index(y, i))) {
					return false;
				}
			}
#pragma warning(pop)

			return true;
		}
		// if (xltypeSRef == type(x))
		//	return x.val.sref.ref == y.val.sref.ref;
		if (xltypeInt == type(x)) {
			return x.val.w == y.val.w;
		}

		return true;
	}

	template<class X>
		requires is_xloper<X>
	inline constexpr X XErr(WORD type)
	{
		return X{ .val = {.err = type}, .xltype = xltypeErr };
	}

	// xlerrX, Excel error string, error description
#define XLL_ERR(X)                                                          \
	X(Null,  "#NULL!",  "intersection of two ranges that do not intersect") \
	X(Div0,  "#DIV/0!", "formula divides by zero")                          \
	X(Value, "#VALUE!", "variable in formula has wrong type")               \
	X(Ref,   "#REF!",   "formula contains an invalid cell reference")       \
	X(Name,  "#NAME?",  "unrecognised formula name or text")                \
	X(Num,   "#NUM!",   "invalid number")                                   \
	X(NA,    "#N/A",    "value not available to a formula.")                \

	// ErrNull4, ...
	#define XLL_ERR4(a, b, c) static constexpr XLOPER Err##a##4 = XErr<XLOPER>(xlerr##a);
	XLL_ERR(XLL_ERR4)
	#undef XLL_ERR4
	// ErrNull, ...
	#define XLL_ERR12(a, b, c) static constexpr XLOPER12 Err##a = XErr<XLOPER12>(xlerr##a);
	XLL_ERR(XLL_ERR12)
	#undef XLL_ERR12

	// Err::NA = xlerrNA, ...
	enum class Err {
	#define XLL_ERR_ENUM(a, b, c) a = xlerr##a,
		XLL_ERR(XLL_ERR_ENUM)
	#undef XLL_ERR_ENUM
	};

	template<class X>
	inline constexpr X XMissing{ .val = { .num = 0 }, .xltype = xltypeMissing };
	inline constexpr XLOPER Missing4 = XMissing<XLOPER>;
	inline constexpr XLOPER12 Missing = XMissing<XLOPER12>;

	template<class X>
	inline constexpr X XNil{ .val = { .num = 0 }, .xltype = xltypeNil};
	inline constexpr XLOPER Nil4 = XNil<XLOPER>;
	inline constexpr XLOPER12 Nil = XNil<XLOPER12>;

#pragma region XOPER
	// value type for XLOPER
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
		XOPER(XOPER&& o)
		{
			xltype = std::exchange(o.xltype, xltypeNil);
			std::swap(val, o.val);
		}
		XOPER& operator=(const X& x)
		{
			XOPER o(x);
			swap(o);

			return *this;
		}
		XOPER& operator=(const XOPER& x)
		{
			return operator=((X)x);
		}
		XOPER& operator=(XOPER&& o) noexcept
		{
			if (this != &o) {
				xltype = std::exchange(o.xltype, xltypeNil);
				std::swap(val, o.val);
			}

			return *this;
		}
		~XOPER()
		{
			free_oper();
		}

		[[nodiscard]] int type() const
		{
			return ::type(*this);
		}

		void swap(XOPER& x) noexcept
		{
			using std::swap;

			swap(xltype, x.xltype);
			swap(val, x.val);
		}

		bool operator==(const X& x) const noexcept
		{
			return ::equal(*this, x);
		}
		bool operator==(const XOPER& o) const
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
#pragma endregion Num

#pragma region Str
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
		XOPER& operator=(const xchar* str)
		{
			XOPER o(str);
			swap(o); // move???

			return *this;
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
#pragma endregion Str

#pragma region Bool
		explicit XOPER(bool xbool)
			: X{.val = {.xbool = xbool}, .xltype = xltypeBool}
		{ }
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
			for (int i = 0; i < size(); ++i) {
				new (val.array.lparray + i)XOPER{};
			}
		}
		XOPER& resize(xrw r, xcol c)
		{
			if (xltypeMulti == xltype) {
				realloc_multi(r, c);
			}
			else {
				XOPER<X> o0{ *this };
				malloc_multi(r, c);
				operator[](0) = o0;
			}

			return *this;
		}
		// stack vertically if same number of columns
		XOPER& stack(const X& x)
		{
			if (xltypeNil == type()) {
				operator=(x);
			}
			else {
				if (overlap(x)) {
					stack(XOPER<X>(x));
				}
				else {
					if (xltypeMulti != type()) {
						auto o0{ *this };
						malloc_multi(1, 1);
						operator[](0) = o0;
					}
					ensure(columns() == ::columns(x));
					auto r = rows();
					resize(r + ::rows(x), columns());
					std::copy(::begin(x), ::end(x), begin() + r * columns());
				}
			}

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
		bool equal(const xchar* str, int len) const noexcept
		{
			if (val.str[0] != len) {
				return false;
			}

			return std::equal(str, str + len, val.str + 1);
		}
		// true if memory overlaps with x
		bool overlap(const X& x) const
		{
			return (begin() <= xll::begin(x) and xll::begin(x) < end())
				|| (begin() < xll::end(x) and xll::end(x) <= end());
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
			ensure(xltypeStr == type());

			if (xlbitXLFree & xltype) {
				X* x[1] = { (X*)this };
				traits<X>::Excelv(xlFree, 0, 1, x);
				xltype = xltypeNil;
			}
			else if (!(xlbitDLLFree & xltype)) {
				::free(val.str);
				xltype = xltypeNil;
			}
			// else let xlAutoFree to its job
		}
		void malloc_multi(xrw r, xcol c)
		{
			val.array.rows = r;
			val.array.columns = c;
			if (size()) {
				val.array.lparray = (X*)malloc(r * c * sizeof(X));
				ensure(val.array.lparray);
				for (int i = 0; i < size(); ++i) {
					new (val.array.lparray + i) XOPER{};
				}

			}
			else {
				val.array.lparray = nullptr;
			}
			xltype = xltypeMulti;
		}
		void realloc_multi(xrw r, xcol c)
		{
			ensure(xltypeMulti == xltype);
			auto n = size(); // old size
			val.array.rows = r;
			val.array.columns = c;

			if (!size()) {
				free_multi();
			}
			else {
				if (n > size()) {
					std::for_each(end(), begin() + n, [](auto& o) { o.free_oper(); });
				}
				else if (n < size()) {
					val.array.lparray = (X*)::realloc(val.array.lparray, size() * sizeof(X));
					ensure(val.array.lparray);
					for (int i = n; i < size(); ++i) {
						new (val.array.lparray + i) XOPER{};
					}
				}
			}
		}
		void free_multi()
		{
			ensure(xltypeMulti == type());
			if (xlbitXLFree & xltype) {
				X* x[1] = { (X*)this };
				traits<X>::Excelv(xlFree, 0, 1, x);
				xltype = xltypeNil;
			}
			else if (!(xlbitDLLFree&xltype)) {
				if (size()) {
					std::for_each(begin(), end(), [](auto& o) { o.free_oper(); });
					::free(val.array.lparray);
				}
				xltype = xltypeNil;
			}
			// else let xlAutoFree do its job
		}
	};
#pragma endregion XOPER
	using OPER4 = XOPER<XLOPER>;
	using OPER = XOPER<XLOPER12>;
/*
	inline const XLOPER True4 = XOPER<XLOPER>(true);
	inline XLOPER False4 = XOPER<XLOPER>(false);
	inline XLOPER12 True = XOPER<XLOPER12>(true);
	inline XLOPER12 False = XOPER<XLOPER12>(false);
*/
}
