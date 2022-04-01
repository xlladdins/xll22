// defines.h - top level include
// Copyright (c) KALX, LLC. All rights reserved. No warranty made.
#pragma once

#ifndef XLL_VERSION
#define XLL_VERSION 12
#endif

#if XLL_VERSION == 12
#define XLOPERX XLOPER12 // Excel 2007 and later
#undef _MBCS
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif
#elif XLL_VERSION == 4
#define XLOPERX XLOPER   // pre Excel 2007
#ifndef _MBCS
#define _MBCS
#endif
#undef _UNICODE
#undef UNICODE
#else
static_assert("XLL_VERSION must be either 4 or 12")
#endif

#include <map>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include "XLCALL.H"

// xltypeX, XLOPERX::val.X, xX, XLL_X, desc
#define XLL_TYPE_SCALAR(X) \
    X(Num,     num,      num,  DOUBLE,  "IEEE 64-bit floating point")          \
    X(Bool,    xbool,    bool, BOOL,    "Boolean value")                       \
    X(Err,     err,      err,  WORD,    "Error type")                          \
    X(SRef,    sref.ref, ref,  LPOPER,  "Single refernce")                     \
    X(Int,     w,        int,  LONG,    "32-bit signed integer")               \
    X(Ref,  mref.lpmref, lpmref, LPOPER,  "Multiple reference")                \

#define X_SCALAR(a, b, c, d, e) | xltype##a
inline constexpr int xltypeScalar = 0 XLL_TYPE_SCALAR(X_SCALAR);
#undef X_SCALAR

// types requiring allocation where xX is pointer to data
// xltypeX, XLOPERX::val.X, xX, XLL_X, desc
#define XLL_TYPE_ALLOC(X) \
    X(Str,     str,     str, PSTRING, "Pointer to a counted Pascal string")    \
    X(Multi,   array,   multi, LPOPER,  "Two dimensional array of OPER types") \
    X(BigData, bigdata.h.lpbData, bigdata, LPOPER,  "Blob of binary data")     \

#define X_ALLOC(a, b, c, d, e) | xltype##a
inline constexpr int xltypeAlloc = 0 XLL_TYPE_ALLOC(X_ALLOC);
#undef X_ALLOC

// xllbitX, desc
#define XLL_BIT(X) \
	X(XLFree,  "Excel owns memory")    \
	X(DLLFree, "xlAutoFree owns memory") \

inline constexpr int xlbitmask = ~(xlbitXLFree | xlbitDLLFree);

#define XLL_NULL_TYPE(X)                    \
	X(Missing, "missing function argument") \
	X(Nil,     "empty cell")                \

#define X_NULL(a, b) template<class X> struct X##a : public X \
{ constexpr X##a() : X{ .val = {.num = 0}, .xltype = xltype##a } { } }; \
inline constexpr XLOPERX a = X##a<XLOPERX>{};
XLL_NULL_TYPE(X_NULL)
#undef X_NULL
#define X_NULL(a,b) inline constexpr XLOPER a##4 = X##a<XLOPER>{};
XLL_NULL_TYPE(X_NULL)
#undef X_NULL
#define X_NULL(a,b) inline constexpr XLOPER12 a##12 = X##a<XLOPER12>{};
XLL_NULL_TYPE(X_NULL)
#undef X_NULL

// xlerrX, Excel error string, error description
#define XLL_ERR(X)                                                          \
	X(Null,  "#NULL!",  "intersection of two ranges that do not intersect") \
	X(Div0,  "#DIV/0!", "formula divides by zero")                          \
	X(Value, "#VALUE!", "variable in formula has wrong type")               \
	X(Ref,   "#REF!",   "formula contains an invalid cell reference")       \
	X(Name,  "#NAME?",  "unrecognised formula name or text")                \
	X(Num,   "#NUM!",   "invalid number")                                   \
	X(NA,    "#N/A",    "value not available to a formula.")                \

#define X_ERR(a, b, c) template<class X> struct XErr##a : public X \
{ constexpr XErr##a() : X{.val = {.err = xlerr##a}, .xltype = xltypeErr} { } }; \
inline constexpr XLOPERX Err##a = XErr##a<XLOPERX>{};
XLL_ERR(X_ERR)
#undef X_ERR
#define X_ERR(a, b, c) inline constexpr XLOPER Err##a##4 = XErr##a<XLOPER>{};
XLL_ERR(X_ERR)
#undef X_ERR
#define X_ERR(a, b, c) inline constexpr XLOPER12 Err##a##12 = XErr##a<XLOPER12>{};
XLL_ERR(X_ERR)
#undef X_ERR

#define X_ERR(a, b, c) {xlerr##a, b},
inline const std::map<int, const char*> xll_err_str = { XLL_ERR(X_ERR) };
#undef X_ERR
#define X_ERR(a, b, c) {xlerr##a, c},
inline const std::map<int, const char*> xll_err_desc = { XLL_ERR(X_ERR) };
#undef X_ERR

// Argument types for Excel Functions
// XLL_XXX, Excel4, Excel12, description
#define XLL_ARG_TYPE(X)                                                      \
X(BOOL,     "A", "A",  "short int used as logical")                          \
X(DOUBLE,   "B", "B",  "double")                                             \
X(CSTRING,  "C", "C%", "XCHAR* to C style NULL terminated unicode string")   \
X(PSTRING,  "D", "D%", "XCHAR* to Pascal style byte counted unicode string") \
X(DOUBLE_,  "E", "E",  "pointer to double")                                  \
X(CSTRING_, "F", "F%", "reference to a null terminated unicode string")      \
X(PSTRING_, "G", "G%", "reference to a byte counted unicode string")         \
X(USHORT,   "H", "H",  "unsigned 2 byte int")                                \
X(WORD,     "H", "H",  "unsigned 2 byte int")                                \
X(SHORT,    "I", "I",  "signed 2 byte int")                                  \
X(LONG,     "J", "J",  "signed 4 byte int")                                  \
X(FP,       "K", "K%", "pointer to struct FP")                               \
X(BOOL_,    "L", "L",  "reference to a boolean")                             \
X(SHORT_,   "M", "M",  "reference to signed 2 byte int")                     \
X(LONG_,    "N", "N",  "reference to signed 4 byte int")                     \
X(LPOPER,   "P", "Q",  "pointer to OPER struct (never a reference type)")    \
X(LPXLOPER, "R", "U",  "pointer to XLOPER struct")                           \
X(VOLATILE, "!", "!",  "called every time sheet is recalced")                \
X(UNCALCED, "#", "#",  "dereferencing uncalced cells returns old value")     \
X(VOID,     "",  ">",  "return type to use for asynchronous functions")      \
X(THREAD_SAFE,  "", "$", "declares function to be thread safe")              \
X(CLUSTER_SAFE, "", "&", "declares function to be cluster safe")             \
X(ASYNCHRONOUS, "", "X", "declares function to be asynchronous")             \

#define X_ARG(a,b,c,d) inline const CHAR* const XLL_##a##4 = b;
XLL_ARG_TYPE(X_ARG)
#undef X_ARG

#define X_ARG(a,b,c,d) inline const XCHAR* const XLL_##a##12 = L##c;
XLL_ARG_TYPE(X_ARG)
#undef X_ARG

#if XLL_VERSION == 12
#define X_ARG(a,b,c,d) inline const XCHAR* const XLL_##a = L##c;
#else
#define X_ARG(a,b,c,d) inline const CHAR* const XLL_##a = b;
#endif
XLL_ARG_TYPE(X_ARG)
#undef X_ARG

// 64-bit uses different symbol name decoration
#ifdef _M_X64 
#define XLL_DECORATE(s,n) s
#define XLL_X64(x) x
#define XLL_X32(x)
#else
#define XLL_DECORATE(s,n) "_" s "@" #n
#define XLL_X64(x)	
#define XLL_X32(x) x
#endif

/*
// Function returning a constant value.
#define XLL_CONST(type, name, value, help, cat, topic) \
AddIn xai_ ## name (Function(XLL_##type, XLL_DECORATE("_xll_" #name, 0) , #name) \
.FunctionHelp(help).Category(cat).HelpTopic(topic)); \
extern "C" __declspec(dllexport) type WINAPI xll_ ## name () { return value; }
*/
