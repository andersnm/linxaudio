/*
	support code required by object files created by msvc/c++
*/

//int _fltused = 0;
int __sse2_available = 0;

// found these here:
// https://github.com/levex/LevOS/blob/master/kernel/Lib/cstd.cpp
float __declspec(naked) _CIcos() {
	_asm {
		fcos
		ret
	}
}

float __declspec(naked) _CIsin() {
	_asm {
		fsin
		ret
	}
}

float __declspec(naked) _CIsqrt() {
	_asm {
		fsqrt
		ret
	}
}

// the following came from a pastebin w/top comment: "math.cpp - math functions 08/20/05 (mv)"
// http://pastebin.com/BUJjhht4
void __declspec(naked) _CIacos() {
	__asm {
		fld             st(0)
		fld             st(0)
		fmul
		fld1
		fsubr
		fsqrt
		fxch
		fpatan
		ret
	}
}

float __declspec(naked) _CIatan2() {
	__asm {
		fpatan;
		ret
	}
}

float asintmp = 0;
float __declspec(naked) _CIasin() {
	_asm {
		fst asintmp
		fld st(0)
		fmul
		fchs
		fld1
		fadd
		fsqrt
		fld asintmp
		fdivrp st(1),st
		fld1
		fpatan
		ret
	}
}

float __declspec(naked) _CIfabs() {
	__asm {
		fabs;
		ret;
	}
}

// http://stackoverflow.com/questions/7420927/unresolved-symbols-when-linking-without-c-runtime-with-vs2010
// had a bug, so instead found this:
// http://bbs.demoscene.fr/code/probleme-crinkler-4k-windows/20/?wap2
void __declspec(naked) _CIpow() {
	_asm {
		fxch st(1)
		fyl2x
		fld st(0)
		frndint
		fsubr st(1),st(0)
		fxch st(1)
		fchs
		f2xm1
		fld1
		faddp st(1),st(0)
		fscale
		fstp st(1)
		ret
	}
}

// nanolibc math dummies illuminator/psikorp '99
void __declspec(naked) _CIfmod() {
	_asm {
		fxch
		fprem
		fstp st(1)
		ret
	}
}


/*
Wutils.c of "Funkuhr DCF77"
https://www-user.tu-chemnitz.de/~heha/hs/mein_msvc.htm

// tatsächlich so kompliziert!
double _cdecl _CIexp() {
 _asm{	fldl2e
	fmulp	st(1),st	// st = f * log2 e
	fld	st		// duplizieren
	frndint			// st = ganzzahliger Teil (Rundung mathematisch)
	fsub	st(1),st	// st(1) = gebrochener Teil (± 0,5)
	fxch	st(1)
	f2xm1			// st = 2^gebrochener_Teil-1 (f2xm1 kann nur im Intervall ±1 arbeiten!)
	fld1
	faddp	st(1),st	// 1 addieren: st = 2^gebrochener_Teil
	fscale			// st = 2^gebrochener_Teil * 2^ganzzahliger_Teil
	fxch	st(1)
	fstp	st		// ganzzahligen Teil entfernen
 }
}

double _cdecl _CIfmod() {
 _asm{	fxch	st(1)
 	fprem
	fxch	st(1)
	fstp	st(0)		// verwerfen
}}


*/

/*
this is not recommended:
long __declspec (naked) _ftol2_sse() {
	int a;
	_asm {
		fistp [a]
		mov	ebx, a
		ret
	}
}

// this leaves the fpu stack out of order, critical for /fp:fast :
__declspec(naked) void _ftol2() {
	__asm {
		push        ebp
		mov         ebp,esp
		sub         esp,20h
		and         esp,0FFFFFFF0h
		fld         st(0)
		fst         dword ptr [esp+18h]
		fistp       qword ptr [esp+10h]
		fild        qword ptr [esp+10h]
		mov         edx,dword ptr [esp+18h]
		mov         eax,dword ptr [esp+10h]
		test        eax,eax
		je          integer_QnaN_or_zero
arg_is_not_integer_QnaN:
		fsubp       st(1),st
		test        edx,edx
		jns         positive
		fstp        dword ptr [esp]
		mov         ecx,dword ptr [esp]
		xor         ecx,80000000h
		add         ecx,7FFFFFFFh
		adc         eax,0
		mov         edx,dword ptr [esp+14h]
		adc         edx,0
		jmp         localexit
positive:
		fstp        dword ptr [esp]
		mov         ecx,dword ptr [esp]
		add         ecx,7FFFFFFFh
		sbb         eax,0
		mov         edx,dword ptr [esp+14h]
		sbb         edx,0
		jmp         localexit
integer_QnaN_or_zero:
		mov         edx,dword ptr [esp+14h]
		test        edx,7FFFFFFFh
		jne         arg_is_not_integer_QnaN
		fstp        dword ptr [esp+18h]
		fstp        dword ptr [esp+18h]
localexit:
		leave
		ret
	}
}
*/
