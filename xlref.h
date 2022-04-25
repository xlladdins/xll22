// xlref.h - Single reference
#pragma once
#include "defines.h"

inline bool operator==(const XLREF& x, const XLREF& y)
{
	return x.rwFirst == y.rwFirst
		&& x.rwLast == y.rwLast
		&& x.colFirst == y.colFirst
		&& x.colLast == y.colLast;
}

inline bool operator==(const XLREF12& x, const XLREF12& y)
{
	return x.rwFirst == y.rwFirst
		&& x.rwLast == y.rwLast
		&& x.colFirst == y.colFirst
		&& x.colLast == y.colLast;
}

namespace xll {

	inline const XLREF* begin(const XLMREF& x)
	{
		return x.reftbl;
	}
	inline const XLREF* end(const XLMREF& x)
	{
		return x.reftbl + x.count;
	}
	inline const XLREF12* begin(const XLMREF12& x)
	{
		return x.reftbl;
	}
	inline const XLREF12* end(const XLMREF12& x)
	{
		return x.reftbl + x.count;
	}

#pragma region XREF
	template<class X>
//		requires is_xloper<X>
	struct XREF : traits<X>::xref {
		using traits<X>::xref::rwFirst;
		using traits<X>::xref::rwLast;
		using traits<X>::xref::colFirst;
		using traits<X>::xref::colLast;
		using xrw = traits<X>::xrw;
		using xcol = traits<X>::xcol;
		XREF(const X& x) noexcept
			: X{ x }
		{ }
		XREF(xrw r, xcol c, xrw h = 1, xcol w = 1) noexcept
			: traits<X>::xref{ 
				.rwFirst = r, .rwLast = static_cast<xrw>(r + h - 1),
				.colFirst = c, .colLast = static_cast<xcol>(c + w - 1) 
			}
		{ }
		auto height() const noexcept 
		{
			return rwLast - rwFirst + 1;
		}
		auto width() const noexcept
		{
			return colLast - colFirst + 1;
		}
		bool operator==(const XREF& x) const
		{
			return ::operator==(*this, x);
		}
		// same as Excel OFFSET()
		XREF& offset(xrw r, xcol c, xrw h = 0, xcol w = 0)
		{
			rwFirst += r;
			rwLast = (h ? rwFirst + h - 1 : rwFirst);
			colFirst += c;
			colLast = (w ? colFirst + w - 1 : colFirst);

			return *this;
		}
		XREF& resize(xrw h, xcol w)
		{
			return offset(0, 0, h, w);
		}
	};
#pragma endregion XREF

} // namespace xll


