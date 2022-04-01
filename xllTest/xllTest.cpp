#include "pch.h"
#include "CppUnitTest.h"
#define XLL_VERISION 4
#include "../xll.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace xll;

template<>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString(const XOPER<XLOPER>& o)
{
	return L"";
}
template<>
inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString(const XOPER<XLOPER12>& o)
{
	return L"";
}

namespace xllTest
{
	TEST_CLASS(_count)
	{
		TEST_METHOD(_c2p)
		{
			Assert::IsNull(c2p<int>(nullptr));
			Assert::IsNull(p2c<int>(nullptr));

			int i[] = { 1,2,3,0 };
			c2p(i);
			Assert::AreEqual(3, i[0]);
			Assert::AreEqual(1, i[1]);
			Assert::AreEqual(2, i[2]);
			Assert::AreEqual(3, i[3]);
			p2c(i);
			Assert::AreEqual(1, i[0]);
			Assert::AreEqual(2, i[1]);
			Assert::AreEqual(3, i[2]);
			Assert::AreEqual(0, i[3]);
		}
	};

	TEST_CLASS(_strings)
	{
	public:
		TEST_METHOD(_wc2mb)
		{
			wchar_t ws[] = L"\x06" L"abc你好";
			char* s = wc2mb(ws);
			Assert::IsTrue(0 == strncmp(s + 1, "abc你好", s[0]));
			delete [] s;
		}
		TEST_METHOD(_mb2wc)
		{
			char s[] = "\x06" "abc你好";
			wchar_t* ws = mb2wc(s);
			Assert::IsTrue(0 == wcsncmp(ws + 1, L"abc你好", ws[0]));
			delete[] ws;
		}
	};

	TEST_CLASS(_XLREF)
	{
		TEST_METHOD(_constructors)
		{
			{
				REF4 r(1, 2);
				auto i = r.rwFirst;
				Assert::IsTrue(1 == r.rwFirst);
				Assert::IsTrue(1 == r.rwLast);
				Assert::IsTrue(2 == r.colFirst);
				Assert::IsTrue(2 == r.colLast);
				Assert::IsTrue(1 == r.height());
				Assert::IsTrue(1 == r.width());
			}
			{
				REF12 r(1, 2);
				auto i = r.rwFirst;
				Assert::IsTrue(1 == r.rwFirst);
				Assert::IsTrue(1 == r.rwLast);
				Assert::IsTrue(2 == r.colFirst);
				Assert::IsTrue(2 == r.colLast);
				Assert::IsTrue(1 == r.height());
				Assert::IsTrue(1 == r.width());
			}

		}
	};

	TEST_CLASS(_XLOPERX)
	{
	public:
		template<class X>
		void xtype()
		{
			X x = { .val = {.num = 1 }, .xltype = xltypeNum };
			Assert::AreEqual(xltypeNum, xll::type(x));
		}
		TEST_METHOD(type)
		{
			xtype<XLOPER>();
			xtype<XLOPERX>();
		}
	};

	TEST_CLASS(_XOPER)
	{
	public:
		template<class X>
		void XDefaultConstructor()
		{
			XOPER<X> o;
			Assert::AreEqual(xltypeNil, o.type());
			XOPER<X> o2{ o };
			Assert::AreEqual(o2, o);
			o = o2;
			Assert::IsTrue(!(o != o2));
		}
		TEST_METHOD(DefaultConstructor)
		{
			XDefaultConstructor<XLOPER>();
			XDefaultConstructor<XLOPER12>();
		}
	};
}
