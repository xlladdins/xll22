// json.h - JSON like OPER
#include <initializer_list>
#include <map>
#include "xloper.h"

namespace xll {

	enum class JSONtype {
		Null,
		False,
		True,
		Number,
		String,
		Array,
		Object,
		Error, // not a JSON type
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
		explicit XJSON(const std::initializer_list<XJSON>& array)
			: XOPER<X>(static_cast<xrw>(array.size()), 1)
		{ 
			for (int i = 0; i < size(); ++i) {
				operator[](i) = *(array.begin() + i);
			}
		}
		explicit XJSON(const std::map<const xchar*,XJSON<X>>& object)
			: XOPER<X>(static_cast<xrw>(object.size()), 2)
		{
			for (int i = 0; i < size(); ++i) {
				operator()(i, 0) = object[i].first;
				operator()(i, 1) = object[i].second;
			}
		}
		auto size() const noexcept
		{
			return XOPER<X>::rows(); // Null has size 0???
		}
		JSONtype jstype() const noexcept
		{
			switch (XOPER<X>::type()) {
			case xltypeNil:
				return JSONtype::Null;
			case xltypeBool:
				return XOPER<X>::val.xbool ? JSONtype::True : JSONtype::False;
			case xltypeNum:
				return JSONtype::Number;
			case xltypeStr:
				return JSONtype::String;
			case xltypeMulti:
				return XOPER<X>::columns() == 1 ? JSONtype::Array
					 : XOPER<X>::columns() == 2 ? JSONtype::Object
					 : JSONtype::Error;
			}

			return JSONtype::Error;
		}
		XJSON& operator[](int i)
		{
			ensure(JSONtype::Array == jstype());
			return static_cast<XJSON&>(XOPER<X>::operator[](i));
		}
		const XJSON& operator[](int i) const
		{
			ensure(JSONtype::Array == jstype());

			return XOPER<X>::operator[](i);
		}
		XJSON& operator()(const xchar* key)
		{
			ensure(JSONtype::Object == jstype());

			// first match
			for (int i = 0; i < size(); ++i) {
				if (operator()(i, 0) == key) {
					return XOPER<X>::operator()(i, 1);
				}
			}
		}
	};

}