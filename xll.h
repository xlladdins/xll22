#pragma once
#include <initializer_list>
#include <vector>
#include "oper.h"
#include "ensure.h"

namespace xll {

	//inline std::map<std::string, int> AddInMap;

	/*
	using cstr = std::string;
	struct arg {
		cstr type;
		cstr name;
		cstr help;
		cstr init;
	};
	struct args {
		cstr type; // return type
		cstr procedure; // name of function to load
		cstr function_text; // name to use
		std::vector<arg> arguments;
		cstr function_help;
		cstr category;
		cstr help_topic;
		cstr documentation;
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
		.function_help = "",
		.category = "",
		.help_topic = "",
	};
	*/
	using cstr = std::string;
	template<class X>
	struct XArg {
		cstr type;
		cstr name;
		cstr help;
		cstr init;
	};

	// arguments of xlfRegister
	template<class X = XLOPER12>
	struct XArgs {
		XOPER<X> file_text; // name of dll
		XOPER<X> procedure; // name of function to load
		XOPER<X> type_text; // signature
		XOPER<X> function_text; // Excel name
		XOPER<X> argument_text; // comma separated argument names
		XOPER<X> macro_type; // 1 for a function or 2 for a command
		XOPER<X> category;
		XOPER<X> shortcut_text;
		XOPER<X> help_topic;
		XOPER<X> function_help;
		XOPER<X> argument_help[21]; int nargs = 0;
		X* opers[32];
		XArgs()
			: file_text{module_text},
			  opers{ 
				&file_text, &procedure, &type_text, &function_text, &argument_text, 
				&macro_type, &category, &shortcut_text, &help_topic, &function_help
			  }
		{
			for (int i = 0; i < 21; ++i) {
				opers[i + 10] = &argument_help[i];
			}
		}
		XArgs& Arguments(std::initializer_list<XArg<X>>& args)
		{
			ensure(args.size() <= 21);
			nargs = static_cast<int>(args.size());

			std::string comma("");
			for (size_t i = 0; i < args.size(); ++i) {
				type_text.append(args[i].type);
				argument_text.append(comma);
				argument_text.append(args[i].name);
				argument_help[i] = args[i].help;
				comma = ", ";
			}

			return *this;
		}
		int ArgCount() const
		{
			return nargs;
		}
		XArgs& Category(const char* category_)
		{
			category = category_;

			return *this;
		}
		const XOPER<X>& Category() const
		{
			return category;
		}
		XArgs& FunctionHelp(const char* function_help_)
		{
			function_help = function_help_;

			return *this;
		}
		const XOPER<X>& FunctionHelp() const
		{
			return function_help;
		}
	};
	using Args4 = XArgs<XLOPER>;
	using Args = XArgs<XLOPER12>;
	
	template<class X>
	struct XMacro : public XArgs<X> {
		XMacro(const char* procedure, const char* function_text)
			: XArgs<X>{}
		{
			XArgs<X>::procedure = procedure;
			XArgs<X>::function_text = function_text;
			XArgs<X>::macro_type = XOPER<X>(2.);
		}

	};
	template<class X>
	struct XFunction : public XArgs<X> {
		XFunction(const char* type_text, const char* procedure, const char* function_text)
			: XArgs()
		{
			XArgs<X>::procedure = procedure;
			XArgs<X>::type_text = type_text;
			XArgs<X>::function_text = function_text;
			XArgs<X>::macro_type = XOPER<X>(1.);
		}

	};
	

	// https://docs.microsoft.com/en-us/office/client-developer/excel/xlfregister-form-1
	inline int Register(Args4& args)
	{
		XLOPER x;
		int ret = Excel4v(xlfRegister, &x, 10 + args.nargs, args.opers);
		return ret;
	}
	inline int Register(Args& args)
	{
		XLOPER12 x;
		int ret = Excel12v(xlfRegister, &x, 10 + args.nargs, args.opers);
		return ret;
	}

} // namespace xll