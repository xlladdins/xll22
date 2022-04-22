// fms_ctypes.h - C data types
#pragma once
#include <string>
#include <vector>

// type, mangled, name
#define CTYPE_ENUM(X) \
	X(signed char,    C, SCHAR) \
	X(char,           D, CHAR) \
	X(unsigned char,  E, UCHAR) \
	X(short,          F, SHORT) \
	X(unsigned short, G, USHORT) \
	X(int,            H, INT) \
	X(unsigned int,   I, UINT) \
	X(long,           J, LONG) \
	X(unsigned long , K, ULONG) \
	X(float,          M, FLOAT) \
	X(double,         N, DOUBLE) \
	X(long double     O, LONG_DOUBLE) \
	X(void*,          P, POINTER) \
	X(array,          Q, ARRAY) \
	X(struct,         V, STRUCT) \
	X(class,          V, CLASS) \
	X(void,           X, VOID) \
//	X(...,            Z, ELLIPSIS) \

namespace fms {

	enum ctype {
#define CTYPE_ENUMX(a, b, c) CTYPE_##c,
		CTYPE_ENUM(CTYPE_ENUMX)
#undef CTYPE_ENUMX
	};

	// function arguments
	using cstr = std::string;
	struct arg {
		ctype type; // argument type
		cstr  name;
		cstr  help;
	};
	// function markup
	struct args {
		cstr module; // full name of library
		ctype type; // return type
		cstr  name; // C name of function to load
		cstr  help; // short description
		std::vector<arg> arguments;
	};

}
