// defines.h - top level definitions
#pragma once
#include <map>
#include "traits.h"

namespace xll {

	template<class X> requires is_xloper<X>
	constinit X XMissing{ /*.val = {.num = 0},*/ .xltype = xltypeMissing};
	inline const XLOPER Missing4 = XMissing<XLOPER>;
	inline const XLOPER12 Missing = XMissing<XLOPER12>;

	template<class X> requires is_xloper<X>
	inline constexpr X XNil{ .val = {.num = 0 }, .xltype = xltypeNil };
	inline constexpr XLOPER Nil4 = XNil<XLOPER>;
	inline constexpr XLOPER12 Nil = XNil<XLOPER12>;

	// fixed size xltypeStr
	template<class X, size_t N>
		requires is_xloper<X>
	class XSTR : public X {
		using xchar = traits<X>::xchar;
		xchar buf[N];
	public:
		XSTR(const xchar(&str)[N])
			: X{ .val = {.str = (xchar*)buf}, .xltype = xltypeStr }
		{
			buf[0] = N - 1;
			std::copy(str, str + N - 1, buf + 1);
		}
	};
#define XLL_STR4(s) XSTR<XLOPER, _countof(s)>(s)
#define XLL_STR(s) XSTR<XLOPER12, _countof(s)>(s)

	template<class X> requires is_xloper<X>
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
	#define XLL_ERR4(a, b, c) inline constexpr XLOPER Err##a##4 = XErr<XLOPER>(xlerr##a);
		XLL_ERR(XLL_ERR4)
	#undef XLL_ERR4
	// ErrNull, ...
	#define XLL_ERR12(a, b, c) inline constexpr XLOPER12 Err##a = XErr<XLOPER12>(xlerr##a);
		XLL_ERR(XLL_ERR12)
	#undef XLL_ERR12

	// Err::NA = xlerrNA, ...
	enum class Err {
	#define XLL_ERR_ENUM(a, b, c) a = xlerr##a,
		XLL_ERR(XLL_ERR_ENUM)
	#undef XLL_ERR_ENUM
	};

	inline const std::map<int, std::string> err_name = {
	#define XLL_ERR_MAP(a, b, c) {xlerr##a, std::string(b)},
			XLL_ERR(XLL_ERR_MAP)
	#undef XLL_ERR_MAP
	};
	inline const std::map<int, std::string> err_desc = {
	#define XLL_ERR_MAP(a, b, c) {xlerr##a, std::string(c)},
			XLL_ERR(XLL_ERR_MAP)
	#undef XLL_ERR_MAP
	};

#undef XLL_ERR

	// Argument types for Excel Functions
	// XLL_XXX, Excel4, Excel12, description
#define XLL_ARG(X)                                                               \
	X(BOOL,     "A", "A",  "short int used as logical")                          \
	X(DOUBLE,   "B", "B",  "double")                                             \
	X(CSTRING,  "C", "C%", "XCHAR* to C style NULL terminated unicode string")   \
	X(PSTRING,  "D", "D%", "XCHAR* to Pascal style byte counted unicode string") \
	X(DOUBLE_,  "E", "E",  "pointer to double")                                  \
	X(CSTRING_, "F", "F%", "reference to a null terminated unicode string")      \
	X(PSTRING_, "G", "G%", "reference to a byte counted unicode string")         \
	X(USHORT,   "H", "H",  "unsigned 2 byte int")                                \
	X(WORD,     "H", "H",  "unsigned 2 byte int")                                \
	X(SHORT,    "I", "I",  "signed 2 byte int")                                  \
	X(LONG,     "J", "J",  "signed 4 byte int")                                  \
	X(FP,       "K", "K%", "pointer to struct FP")                               \
	X(BOOL_,    "L", "L",  "reference to a boolean")                             \
	X(SHORT_,   "M", "M",  "reference to signed 2 byte int")                     \
	X(LONG_,    "N", "N",  "reference to signed 4 byte int")                     \
	X(LPOPER,   "P", "Q",  "pointer to OPER struct (never a reference type)")    \
	X(LPXLOPER, "R", "U",  "pointer to XLOPER struct")                           \
	X(VOLATILE, "!", "!",  "called every time sheet is recalced")                \
	X(UNCALCED, "#", "#",  "dereferencing uncalced cells returns old value")     \
	X(VOID,     "",  ">",  "return type to use for asynchronous functions")      \
	X(THREAD_SAFE,  "", "$", "declares function to be thread safe")              \
	X(CLUSTER_SAFE, "", "&", "declares function to be cluster safe")             \
	X(ASYNCHRONOUS, "", "X", "declares function to be asynchronous")             \

	// XLL_BOOL4, ...
	#define XLL_ARG_TYPE(a, b, c, d) inline const char XLL_##a##4[] = b;
		XLL_ARG(XLL_ARG_TYPE)
	#undef XLL_ARG_TYPE
	// XLL_BOOL, ...
	#define XLL_ARG_TYPE(a, b, c, d) inline const char XLL_##a[] = c;
		XLL_ARG(XLL_ARG_TYPE)
	#undef XLL_ARG_TYPE
	#undef XLL_ARG

// 64-bit uses different symbol name decoration
#ifdef _M_X64 
#define XLL_DECORATE(s,n) s
#define XLL_X64(x) x
#define XLL_X32(x)
#else
#define XLL_DECORATE(s,n) "_" s "@" #n
#define XLL_X64(x)	
#define XLL_X32(x) x
#endif

} // namespace xll