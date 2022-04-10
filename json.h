// json.h - JSON like OPER
#include <initializer_list>
#include <map>
#include "xloper.h"

namespace xll {

	enum class JSON_t {
		Null,
		False,
		True,
		Number,
		String,
		Array,
		Object,
		Undefined, // not a JSON type
	};

	template<class X>
	struct XJSON : protected XOPER<X> {
	public:
		using XOPER<X>::val;
		using XOPER<X>::xltype;
		using XOPER<X>::operator[];
		using XOPER<X>::operator();

		using xchar = traits<X>::xchar;
		using xrw = traits<X>::xrw;

		// Nil corresponds to null
		XJSON() noexcept
			: XOPER<X>()
		{ }
		explicit XJSON(bool xbool)
			: XOPER<X>(xbool)
		{ }
		explicit XJSON(double num)
			: XOPER<X>(num)
		{ }
		template<class T>
		XJSON(T t)
			: XJSON(static_cast<double>(t))
		{ }
		explicit XJSON(const xchar* str)
			: XOPER<X>(str)
		{ }
		// array
		XJSON& append(const XJSON& o)
		{
			ensure(JSON_t::Object != jstype());

			if (JSON_t::Null == jstype()) {
				XOPER<X>::resize(1, 1);
			}
			else {
				XOPER<X>::resize(size() + 1, 1);
			}
			operator[](size() - 1) = o;

			return *this;
		}

		auto size() const noexcept
		{
			return XOPER<X>::rows(); // Null has size 0???
		}
		JSON_t jstype() const noexcept
		{
			switch (XOPER<X>::type()) {
			case xltypeNil:
				return JSON_t::Null;
			case xltypeBool:
				return XOPER<X>::val.xbool ? JSON_t::True : JSON_t::False;
			case xltypeNum:
				return JSON_t::Number;
			case xltypeStr:
				return JSON_t::String;
			case xltypeMulti:
				return XOPER<X>::columns() == 1 ? JSON_t::Array
					 : XOPER<X>::columns() == 2 ? JSON_t::Object
					 : JSON_t::Undefined;
			}

			return JSON_t::Undefined;
		}
		XJSON& operator[](int i)
		{
			ensure(JSON_t::Array == jstype());
			return static_cast<XJSON&>(XOPER<X>::operator[](i));
		}
		const XJSON& operator[](int i) const
		{
			ensure(JSON_t::Array == jstype());

			return XOPER<X>::operator[](i);
		}
		XJSON& operator()(const xchar* key)
		{
			ensure(JSON_t::Object == jstype());

			// first match
			for (int i = 0; i < size(); ++i) {
				if (operator()(i, 0) == key) {
					return XOPER<X>::operator()(i, 1);
				}
			}
		}
	};

}