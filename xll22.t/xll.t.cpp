#include <cassert>
#include "../xll.h"

using namespace xll;


int main()
{
	const XLOPER x = STR4("x");
	const XLOPER12 y = STR12(L"foo");
	const XLOPERX z = STR(_T("foo"));

	return 0;
}

template<class X>
int test_oper_num()
{
	XOPER<X> o(1.23);
	assert(xltypeNum == o.xltype);
	assert(1.23 == o.val.num );
	o = 2.34;

	XOPER<X> o2;
	o2 = o;
	assert(xltypeNum == o2.xltype);
	assert(2.34 == o2.val.num);

	XOPER<X> o3(o2);
	assert(xltypeNum == o3.xltype);
	assert(2.34 == o3.val.num);

	o3 = 3.45;
	o = o3;
	assert(xltypeNum == o.xltype);
	assert(3.45 == o.val.num);

	double num = 2*o + o2;
	assert(2*3.45 + 2.34 == num);

	o3 = 2 * o + o2;
	assert(2 * 3.45 + 2.34 == o3.val.num);

	XOPER<X> o4;
	o4 = 2 * o + o2;
	assert(xltypeNum == o4.xltype);
	assert(2 * 3.45 + 2.34 == o4.val.num);

	return 0;
}

template<class X>
int test_oper()
{
	{
		XOPER<X> o;
		assert(xltypeNil == o.type());
	}

	test_oper_num<X>();

	return 0;
}
int oper_test = test_oper<XLOPER>() + test_oper<XLOPER12>();