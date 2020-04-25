struct thiscall_function {
	void* function;
	int argument_count;
	void* trampoline_function_ptr;
};

struct thiscall_wrap {
	int argument_count;
	void* trampoline_function_ptr;
};

void* thiscall_object_new_vtbl(int function_count, struct thiscall_function* functions);
void* thiscall_object_new_trampolines(int function_count, struct thiscall_function* functions);
void* thiscall_wrap_new_trampolines(int function_count, struct thiscall_wrap* functions);
