#pragma once
#include <vector>
#include "xloper.h"

namespace xll {

	// fixed size xltypeStr
	template<class X, size_t N>
	class XSTR : public X {
		using xchar = traits<X>::xchar;
		xchar buf[N + 1];
	public:
		XSTR(const xchar* str, xchar len)
			: X{ .val = {.str = (xchar*)buf}, .xltype = xltypeStr }
		{
			std::copy(str, str + len, buf + 1);
			buf[0] = len;
		}
	};
#define XLL_STR4(s) XSTR<XLOPER>(s, _countof(s))
#define XLL_STR(s) XSTR<XLOPER12>(s, _countof(s))

	using cstr = std::string;
	struct arg {
		cstr type;
		cstr name;
		cstr help;
		cstr init;
	};
	struct args {
		cstr module;
		cstr type; // return type
		cstr procedure; // name of function to load
		cstr function_text; // name to use
		std::vector<arg> arguments;
		cstr category;
		cstr help_topic;
		cstr function_help;
	};

	inline args xxxa = {
		//.module = "",
		.type = "",
		.procedure = "",
		.function_text = "",
		.arguments = {
			arg("","","",""),
			arg("","","",""),
		},
		.category = "",
		.help_topic = "",
		.function_help = "",
	};

	// arguments of xlfRegister
	template<class X = XLOPER12>
	struct XArgs {
		X file_text; // name of dll
		X procedure; // name of function to load
		X type_text; // signature
		X function_text; // Excel name
		X argument_text; // comma separated argument names
		X macro_type; // 1 for a function or 2 for a command
		X category;
		X shortcut_text;
		X help_topic;
		X function_help;
		X argument_help[21];
		X* opers[32];
		XArgs()
			: file_text{ .val = { .str = const_cast<XCHAR*>(module_text) }, .xltype = xltypeStr },
			  type_text{XNil<X>},
			  argument_text{XNil<X>},
			  category{XNil<X>},
			  shortcut_text{XNil<X>},
			  help_topic{XNil<X>},
			  function_help{XNil<X>},
			  opers{ 
				&file_text, &procedure, &type_text, &function_text, &argument_text, 
				&macro_type, &category, &shortcut_text, &help_topic, &function_help
			  }
		{
			for (int i = 0; i < 21; ++i) {
				argument_help[i] = XNil<X>;
				opers[i + 10] = &argument_help[i];
			}
		}
		// macro
		XArgs(const X& procedure, const X& function_text)
			: XArgs()
		{
			this->procedure = procedure;
			this->function_text = function_text;
			macro_type = XNum<X>(2);
		}
		// function
		XArgs(const X& type_text, const X& procedure, const X& function_text)
			: XArgs()
		{
			this->procedure = procedure;
			this->type_text = type_text;
			this->function_text = function_text;
			macro_type = XNum<X>(1);
		}
		// function
	};
	using Args4 = XArgs<XLOPER>;
	using Args = XArgs<XLOPER12>;

	inline int Register(Args4& args)
	{
		XLOPER x;
		int ret = Excel4v(xlfRegister, &x, 10/* + n*/, args.opers);
		return ret;
	}
	inline int Register(Args& args)
	{
		XLOPER12 x;
		int ret = Excel12v(xlfRegister, &x, 10, args.opers);
		return ret;
	}

}