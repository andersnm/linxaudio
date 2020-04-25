/*
	thiscall.c - x86 ffi for composing and calling msvc++ class members
	using the default "thiscall" calling convention.
	
	Loosely based on a gist by Kenneth Waters:
	https://gist.github.com/kwaters/1516195
*/
#define _USE_MATH_DEFINES
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <sys/mman.h>
#include "thiscall.h"

static const unsigned char thiscall_function_trampoline_1[] = { 
	// push ebp  
	0x55, 
	// mov ebp, esp
	0x8B, 0xEC
};

static const unsigned char thiscall_function_trampoline_2[] = { 
	// push dword ptr [ebp+0]  
	0xFF, 0x75, 0,
};

static const unsigned char thiscall_function_trampoline_3[] = { 
	// push ecx (this)
	0x51,
	// call rel32 (0)
	0xE8, 0, 0, 0, 0,
	// mov esp,ebp  
	0x8B, 0xE5,
	// pop ebp
	0x5D,
	// ret 0
	0xC2, 0x00, 0x00
};

void* thiscall_object_new_trampolines(int function_count, struct thiscall_function* functions) {
	int i,j;
	int ret;
	int codepage_size = 4096;
	// TODO - why is this? what if there are more than 4k functions
	//int codepage_size = function_count * sizeof(thiscall_function_trampoline);
	unsigned char* codepage = (unsigned char*)mmap(NULL, codepage_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	unsigned char* codepage_ptr = codepage; 

	for (i = 0; i < function_count; i++) {
		int rel;
		unsigned short function_count_short;
		int argument_count = functions[i].argument_count;
		unsigned char* funcdata = (unsigned char*)functions[i].function;
		functions[i].trampoline_function_ptr = codepage_ptr;
		memcpy(codepage_ptr, thiscall_function_trampoline_1, sizeof(thiscall_function_trampoline_1));
		codepage_ptr += sizeof(thiscall_function_trampoline_1);

		for (j = 0; j < argument_count; j++) {
			memcpy(codepage_ptr, thiscall_function_trampoline_2, sizeof(thiscall_function_trampoline_2));
			codepage_ptr[2] = (unsigned char)(char)(8 + (argument_count - j - 1) * 4);
			//codepage_ptr[2] = (unsigned char)(char)(-8 - j*4);
			codepage_ptr += sizeof(thiscall_function_trampoline_2);
		}

		memcpy(codepage_ptr, thiscall_function_trampoline_3, sizeof(thiscall_function_trampoline_3));

		rel = (funcdata - codepage_ptr) - 6;
		memcpy(codepage_ptr + 2, &rel, sizeof(void*));
		
		function_count_short = argument_count * sizeof(void*);
		memcpy(codepage_ptr + 10, &function_count_short, sizeof(unsigned short));

		codepage_ptr += sizeof(thiscall_function_trampoline_3);
	}

	ret = mprotect(codepage, codepage_size, PROT_READ | PROT_EXEC);
	assert(!ret); 
	return codepage;
}

void* thiscall_object_new_vtbl(int function_count, struct thiscall_function* functions) {
	int i;
	void** result = (void**)malloc(sizeof(void*) * function_count);
	for (i = 0; i < function_count; i++) {
		result[i] = (unsigned char*)functions[i].trampoline_function_ptr;
	}
	return result;
}

void thiscall_object_destroy_vtbl(void* vtbl) {
	free(vtbl);
}

void thiscall_object_destroy_trampolines(void* codepage) {
	int codepage_size = 4096;
	int ret = munmap(codepage, codepage_size);
	assert(!ret);
}

// call vtable index w/ecx as this
const unsigned char thiscall_wrap_trampoline_1[] = {
	// push ebp  
	0x55, 
	// mov ebp, esp
	0x8B, 0xEC
};

const unsigned char thiscall_wrap_trampoline_2[] = {
	// push dword ptr [ebp+0]  
	0xFF, 0x75, 0,
};

static const unsigned char thiscall_wrap_trampoline_3[] = { 
	// mov ecx, [epb+0]
	0x8B, 0x4D, 0x00,
	// mov eax, ecx
	0x8B, 0xC1,
	// mov edx, [eax]
	0x8B, 0x10,
	// mov eax, [edx+0]
	0x8B, 0x42, 0x00,
	// call eax
	0xFF, 0xD0,
	// mov esp,ebp  
	0x8B, 0xE5,
	// pop ebp
	0x5D,
	// ret 0
	0xC2, 0x00, 0x00
};

void* thiscall_wrap_new_trampolines(int function_count, struct thiscall_wrap* functions) {
	int i,j;
	int ret;
	int codepage_size = 4096;
	// TODO - why is this? what if there are more than 4k functions
	//int codepage_size = function_count * sizeof(thiscall_function_trampoline);
	unsigned char* codepage = (unsigned char*)mmap(NULL, codepage_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	unsigned char* codepage_ptr = codepage; 

	for (i = 0; i < function_count; i++) {
		unsigned char function_vtbl_index = i;
		functions[i].trampoline_function_ptr = codepage_ptr;
		memcpy(codepage_ptr, thiscall_wrap_trampoline_1, sizeof(thiscall_wrap_trampoline_1));
		codepage_ptr += sizeof(thiscall_wrap_trampoline_1);

		for (j = 0; j < functions[i].argument_count; j++) {
			memcpy(codepage_ptr, thiscall_wrap_trampoline_2, sizeof(thiscall_wrap_trampoline_2));
			codepage_ptr[2] = (unsigned char)(char)(12 + j*4);
			codepage_ptr += sizeof(thiscall_wrap_trampoline_2);
		}

		memcpy(codepage_ptr, thiscall_wrap_trampoline_3, sizeof(thiscall_wrap_trampoline_3));
		codepage_ptr[2] = 8;
		codepage_ptr[9] = function_vtbl_index * sizeof(void*);

		codepage_ptr += sizeof(thiscall_wrap_trampoline_3);
	}

	ret = mprotect(codepage, codepage_size, PROT_READ | PROT_EXEC);
	assert(!ret); 
	return codepage;
}
