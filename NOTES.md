# Notes

// type, mangled, name
#define DATA_TYPE_ENUMX) \
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
X(long,double     O, LONG_DOUBLE) \
X(pointer,        P, POINTER) \
X(array,          Q, ARRAY) \
X(struct,         V, STRUCT) \
X(class,          V, CLASS) \
X(void,           X, VOID) \
X(ellipsis,       Z, ELLIPSIS) \


enum type {
	FFI_DOUBLE, // ...
}

struct arg {
	type type;
	string name;
	string help;
	string init;
};

// platform independent markup
struct ffi {
	type ret;
	void* fun; // function pointer
	string name;
	vector<arg> sig;
	string desc;
	string help;
	string docs;
};

double foo(long i) { return i; }

struct ffi foo_desc = {
	FFI_DOUBLE, foo, "FOO",
	sig = {
		arg(FFI_LONG, "i", "is the argument."),
	}
	"The identity function.",
	"Return the argument passed.",
	R"()", // no help
};

http://www.kegel.com/mangle.html

Here's another shot at a grammar for names mangled by Visual C++, in a different style:
MangledName:
	'?' BasicName Qualification '@' QualifiedTypeCode StorageClass
	'?' BasicName '@' UnqualifiedTypeCode StorageClass

BasicName:
	'?' operatorCode
	string '@'

Qualification:
	( string '@' )* 

QualifiedTypeCode:
	'Q' FunctionTypeCode
	'2' DataTypeCode

UnqualifiedTypeCode:
	'Y' FunctionTypeCode
	'3' DataTypeCode

StorageClass:
	'A' (Normal Storage)
	'B' (Volatile Storage)
	'C' (Const Storage)
	'Z' (Executable Storage)

FunctionTypeCode:
	CallingConvention ReturnValueTypeCode ArgumentList
Operators
Operator names are represented by a question mark followed by a single character rather than being spelled out. Examples:
myclass::myclass          ?0
myclass::~myclass         ?1
myclass::operator new     ?2
myclass::operator delete  ?3
myclass::operator=        ?4
myclass::operator+        ?H
myclass::operator++       ?E
Attributes
These appear immediately before the type they modify.
volatile ?C
const    ?B
Function Calling Convention Codes
__cdecl -> "A"
__fastcall -> "I"
__stdcall -> "G"
Function Argument Type Lists
Function arguments types are listed left to right using the following codes. If the parameter list is not void and does not end in elipsis, its encoding is terminated by an @.
Data Type Codes
signed char    C
char           D
unsigned char  E
short          F
unsigned short G
int            H
unsigned int   I
long           J
unsigned long  K
float          M
double         N
long double    O
pointer        P (see below)
array          Q (see below)
struct/class   V (see below)
void           X (terminates argument list)
elipsis        Z (terminates argument list)
Function Pointers
Here's a few examples that tease out how function pointers work:
typedef int (*x)(int); typedef int (*y)(short);
int Fx_i(x fnptr)	?Fx_i@@YAHP6AHH@Z@Z
int Fxix_i(x fnptr, int i, x fnptr3)	?Fxix_i@@YAHP6AHH@ZH0@Z
int Fxx_i(x fnptr, x fnptr2)	?Fxx_i@@YAHP6AHH@Z0@Z
int Fxxi_i(x fnptr, x fnptr2, x fnptr3, int i)	?Fxxi_i@@YAHP6AHH@Z00H@Z
int Fxxx_i(x fnptr, x fnptr2, x fnptr3)	?Fxxx_i@@YAHP6AHH@Z00@Z
int Fxyxy_i(x fnptr, y fnptr2, x fnptr3, y fnptr4)	?Fxyxy_i@@YAHP6AHH@ZP6AHF@Z01@Z
the signature of the arguments of Fxxxi_i() is P6AHH@Z, 0, 0, H, @ Possibly this is interpreted as follows:
P6 = function pointer
A = __cdecl
H = returns int
H = argument int
@ = end of argument list
Z = end of signature
0 (zero) = repeat first type
0 (zero) = repeat first type
H = argument int
@ = end of argument list
The signature of the arguments of Fxix_i, i.e. (x fnptr, int i, y fnptr2) is "P6AHH@Z, H, 0, @", and the signature of the arguments of Fxyxy_i, i.e. (x fnptr, y fnptr2, x fnptr3, y fnptr4), is "P6AHH@Z, P6AHF@Z, 0, 1, @". Apparantly digits refer back to types referenced previously in this definition; 0 (zero) refers to the first type, and 1 (one) refers back to the second type.
Classes
void Fmyclass_v(myclass m)	?Fmyclass_v@@YAXVmyclass@@@Z
void Fmxmx_v(myclass arg1, x arg2, myclass arg3, x arg4)	?Fmxmx_v@@YAXVmyclass@@P6AHH@Z01@Z
The signature of an argument list (myclass m) is Vmyclass@@@, which breaks down as follows: means
V = class
class name
@@ = end of class name
@ = end of argument list
The signature of an argument list (myclass arg1, x arg2, myclass arg3, x arg4) is Vmyclass@@P6AHH@Z01@, which breaks down as follows:
V = class
class name
@@ = end of class name
P6AHH@Z = signature of x
0 = refer back to first type
1 = refer back to second type
@ = end of argument list
As above, digits refer back to types defined previously.
Examples
Using a simple C++ source file and a perl script, the mangled names for a few functions were generated using Visual C++ 4.2. (The functions were named using something like the mangling scheme in "The Annotated C++ Reference Manual", just for grins.)
typedef int (*x)(int); typedef int (*y)(short);
static int myclass::myStaticMember	?myStaticMember@myclass@@2HA
const int myclass::myconstStaticMember	?myconstStaticMember@myclass@@2HB
volatile int myclass::myvolatileStaticMember	?myvolatileStaticMember@myclass@@2HC
x myfnptr;	?myfnptr@@3P6AHH@ZA
int myglobal;	?myglobal@@3HA
volatile int myvolatile;	?myvolatile@@3HC
int myarray[10];	?myarray@@3PAHA
void **Fv_PPv(void)	?Fv_PPv@@YAPAPAXXZ
void *Fv_Pv(void)	?Fv_Pv@@YAPAXXZ
int FA10_i_i(int a[10])	?FA10_i_i@@YAHQAH@Z
int FPi_i(int *a)	?FPi_i@@YAHPAH@Z
int Fc_i(char bar)	?Fc_i@@YAHD@Z
int Ff_i(float bar)	?Ff_i@@YAHM@Z
int Fg_i(double bar)	?Fg_i@@YAHN@Z
int Fi_i(int bar)	?Fi_i@@YAHH@Z
int Fie_i(int bar, ...)	?Fie_i@@YAHHZZ
int Fii_i(int bar, int goo)	?Fii_i@@YAHHH@Z
int Fiii_i(int bar, int goo, int hoo)	?Fiii_i@@YAHHHH@Z
void Fmxmx_v(myclass arg1, x arg2, myclass arg3, x arg4)	?Fmxmx_v@@YAXVmyclass@@P6AHH@Z01@Z
void Fmyclass_v(myclass m)	?Fmyclass_v@@YAXVmyclass@@@Z
const int Fv_Ci(void)	?Fv_Ci@@YA?BHXZ
long double Fv_Lg(void)	?Fv_Lg@@YAOXZ
int& Fv_Ri(void)	?Fv_Ri@@YAAAHXZ
signed char Fv_Sc(void)	?Fv_Sc@@YACXZ
unsigned char Fv_Uc(void)	?Fv_Uc@@YAEXZ
unsigned int Fv_Ui(void)	?Fv_Ui@@YAIXZ
unsigned long Fv_Ul(void)	?Fv_Ul@@YAKXZ
unsigned short Fv_Us(void)	?Fv_Us@@YAGXZ
volatile int Fv_Vi(void)	?Fv_Vi@@YA?CHXZ
char Fv_c(void)	?Fv_c@@YADXZ
float Fv_f(void)	?Fv_f@@YAMXZ
double Fv_g(void)	?Fv_g@@YANXZ
int Fv_i(void)	?Fv_i@@YAHXZ
long Fv_l(void)	?Fv_l@@YAJXZ
short Fv_s(void)	?Fv_s@@YAFXZ
void Fv_v(void)	?Fv_v@@YAXXZ
void __cdecl Fv_v_cdecl(void)	?Fv_v_cdecl@@YAXXZ
void __fastcall Fv_v_fastcall(void)	?Fv_v_fastcall@@YIXXZ
void __stdcall Fv_v_stdcall(void)	?Fv_v_stdcall@@YGXXZ
int Fx_i(x fnptr)	?Fx_i@@YAHP6AHH@Z@Z
int Fxix_i(x fnptr, int i, x fnptr3)	?Fxix_i@@YAHP6AHH@ZH0@Z
int Fxx_i(x fnptr, x fnptr2)	?Fxx_i@@YAHP6AHH@Z0@Z
int Fxxi_i(x fnptr, x fnptr2, x fnptr3, int i)	?Fxxi_i@@YAHP6AHH@Z00H@Z
int Fxxx_i(x fnptr, x fnptr2, x fnptr3)	?Fxxx_i@@YAHP6AHH@Z00@Z
int Fxyxy_i(x fnptr, y fnptr2, x fnptr3, y fnptr4)	?Fxyxy_i@@YAHP6AHH@ZP6AHF@Z01@Z
void myclass::operator delete(void *p)	??3myclass@@SAXPAX@Z
int myclass::Fi_i(int bar)	?Fi_i@myclass@@QAEHH@Z
static int myclass::Fis_i(int bar)	?Fis_i@myclass@@SAHH@Z
void __cdecl myclass::Fv_v_cdecl(void)	?Fv_v_cdecl@myclass@@QAAXXZ
void __fastcall myclass::Fv_v_fastcall(void)	?Fv_v_fastcall@myclass@@QAIXXZ
void __stdcall myclass::Fv_v_stdcall(void)	?Fv_v_stdcall@myclass@@QAGXXZ
myclass::myclass(int x)	??0myclass@@QAE@H@Z
myclass::myclass(void)	??0myclass@@QAE@XZ
int myclass::nested::Fi_i(int bar)	?Fi_i@nested@myclass@@QAEHH@Z
myclass::nested::nested(void)	??0nested@myclass@@QAE@XZ
myclass::nested::~nested()	??1nested@myclass@@QAE@XZ
myclass myclass::operator+(int x)	??Hmyclass@@QAE?AV0@H@Z
myclass myclass::operator++()	??Emyclass@@QAE?AV0@XZ
myclass myclass::operator++(int)	??Emyclass@@QAE?AV0@H@Z
myclass& myclass::operator=(const myclass& from)	??4myclass@@QAEAAV0@ABV0@@Z
myclass::~myclass()	??1myclass@@QAE@XZ
int nested::Fi_i(int bar)	?Fi_i@nested@@QAEHH@Z
nested::nested(void)	??0nested@@QAE@XZ
nested::~nested()	??1nested@@QAE@XZ
void* myclass::operator new(size_t size)	??2myclass@@SAPAXI@Z
