// xlref.h - Single reference
#pragma once
#include "XLCALL.H"

namespace xll {

	template<class X> struct traits {};
	template<> struct traits<XLREF> {
		using type = XLREF;
	};
	template<> struct traits<XLREF12> {
		using type = XLREF12;
	};

#pragma region XREF
	template<class X>
//		requires is_xloper<X>
	struct XREF : X {
		using traits<X>::xref::rwFirst;
		using traits<X>::xref::rwLast;
		using traits<X>::xref::colFirst;
		using traits<X>::xref::colLast;
		using xrw = traits<X>::xrw;
		using xcol = traits<X>::xcol;
		XREF(const X& x)
			: X{ x }
		{ }
		XREF(xrw r, xcol c, xrw h = 1, xcol w = 1)
			: traits<X>::xref{ .rwFirst = r, .rwLast = r + h - 1, .colFirst = c, .colLast = c + w - 1 }
		{ }
		bool operator==(const XREF& x) const noexcept
		{
			return rwFirst == x.rwFirst
				&& rwLast == x.rwLast
				&& colLast == x.colFirst
				&& colLast == x.colLast;
		}
		// same as Excel OFFSET()
		XREF& offset(xrw r, xcol c, xrw h = 0, xcol w = 0)
		{
			rwFirst += r;
			rwLast += (h ? rwFirst + h - 1 : r);
			colFirst += c;
			colLast += (w ? colFirst + w - 1 : c);

			return *this;
		}
	};
#pragma endregion XREF



} // namespace xll

inline bool operator==(const XLREF& x, const XLREF& y)
{
	return x.rwFirst == y.rwFirst
		&& x.rwLast == y.rwLast
		&& x.colLast == y.colFirst
		&& x.colLast == y.colLast;
}

inline bool operator==(const XLREF12& x, const XLREF12& y)
{
	return x.rwFirst == y.rwFirst
		&& x.rwLast == y.rwLast
		&& x.colLast == y.colFirst
		&& x.colLast == y.colLast;
}


