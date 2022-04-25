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


}
