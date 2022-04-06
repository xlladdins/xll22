// test.cpp
#include "xll.h"

using namespace xll;

int test_test()
{
	{
		auto num = Num(1.23);
	}

	return 0;
}
int test_test_ = test_test();

extern "C" __declspec(dllexport) int WINAPI foo(void)
{
	//XLL_INFO("It worked!");

	return TRUE;
}

template<class X>
void test_oper()
{
	{
		XOPER<X> x;
	}
	{
		OPER4 o3("foo");
		OPER o(L"foo");
	}
}

void test_str()
{
	{
		Str4 s;
		Str4 s2{ s };
		ensure(s == s2);
		ensure(s <= s2);
		ensure(!(s < s2));
		s = s2;
		ensure(!(s != s2));
		ensure(s >= s2);
		ensure(!(s > s2));

		ensure(Str4("abc") == "abc");
		ensure(Str4("bbc") > "abc");
		ensure(Str4("abc") < "abd");
	}
	{
		Str s;
		Str s2{ s };
		ensure(s == s2);
		ensure(s <= s2);
		ensure(!(s < s2));
		s = s2;
		ensure(!(s != s2));
		ensure(s >= s2);
		ensure(!(s > s2));

		ensure(Str(L"abc") == L"abc");
		ensure(Str(L"bbc") > L"abc");
		ensure(Str(L"abc") < L"abd");
	}
	{
		Str4 s("abc");
		Str4 s2{ s };
		ensure(s == s2);
		s = s2;
		ensure(!(s != s2));

		ensure(s == "abc");
		ensure(Str4("bbc") > s);
		ensure(s < "abd");
	}
	{
		Str s(L"abc");
		Str s2{ s };
		ensure(s == s2);
		s = s2;
		ensure(!(s != s2));

		ensure(s == L"abc");
		ensure(Str(L"bbc") > s);
		ensure(s < L"abd");
	}
}
