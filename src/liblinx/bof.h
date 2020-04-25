enum archtype {
	arch_none,
	arch_i386,
	arch_x64,
	arch_arm,
	arch_armv7
};

enum symboltype {
	symbol_none,
	symbol_section,
	symbol_file,
	symbol_func,
	symbol_object
};

struct symbol {
	std::string name;
	symboltype type;
	std::string section;
	int section_index;
	int value;
	int size;
	bool is_global;
	bool is_export;
	std::string import_module;
	std::string import_mangled;
};

enum relocationtype {
	r_32,
	r_pc32,
	r_32_nobase,
	r_32_secrel,
	r_32_section,
	r_64_addr64,
	r_64_addr32nb,
	r_64_rel,
	r_64_rel32_1,
};

struct relocation {
	relocationtype type;
	int offset;
	std::string symbol;
};

struct codebits {
	std::string name;
	int index;
	std::vector<char> bytes;
	std::vector<relocation> relocations;
	bool is_executable;
	bool is_readable;
	bool is_writable;
	bool is_uninitialized;

	codebits() {
		is_executable = false;
		is_readable = false;
		is_writable = false;
		is_uninitialized = false;
	}
};

struct binary_object_file {
	archtype arch;
	std::vector<symbol> symbols;
	std::vector<codebits> code;

	binary_object_file() {
		arch = arch_none;
	}
};

// pe imports in the regular msvc toolchain require heavy duty work from the
// linker: 1) use of dll import libs 2) fancy idata merging 3) eliminate
// unused sections. liblinx support none of this, so until then pe imports
// must be hard coded for OS dlls, like kernel32.dll etc.
// each import has to be defined by a "trigger symbol", ie __imp__XXX, which
// stores the imported function's jump address (populated by the os loader), 
// and accompanied with the actual import name to write in the final 
// executable's import descriptor table.
struct pe_import {
	std::string module_name;	// f.ex kernel32.dll
	std::string import_symbol;  // f.ex HeapAlloc
	std::string trigger_symbol; // f.ex _imp__HeapAlloc@12
};

bool bof_load_elf(const std::string& filename, binary_object_file& result);
bool bof_load_coff(const std::string& filename, binary_object_file& result);
bool bof_try_load_object_file(const std::string& filename, const std::vector<std::string>& search_paths, std::string& result_path, binary_object_file& result);

bool bof_save_pe(const binary_object_file& bof, const std::string& filename, const std::vector<pe_import>& imports);
bool bof_add(binary_object_file& dest, const binary_object_file& src);
codebits* bof_find_code(binary_object_file& bof, const std::string& name, int index);
symbol* bof_find_symbol(binary_object_file& bof, const std::string& name);
relocation* bof_find_relocation_by_offset(codebits& code, int value, int* result_data);
int bof_find_max_relocation_offset(codebits& code);
void bof_get_undefined_symbols(binary_object_file& bof, std::vector<std::string>& result);

void bof_graph_add_relocation(codebits& code, relocationtype type, const std::string& symbol, int offset);
void bof_graph_add_symbol(binary_object_file& bof, bool is_global, const std::string& name, const std::string& section, int section_index, symboltype type, int value, int size, const std::string& import_module = "", const std::string& import_mangled = "");
void bof_add_exports(binary_object_file& bof, const std::vector<std::string>& exports);
const pe_import* bof_find_import(const std::vector<pe_import>& imports, const std::string& trigger_symbol);
bool bof_get_code_and_symbol(binary_object_file& bof, const std::string& name, codebits** resultcode, symbol** resultsym);

void bof_add_imports(binary_object_file& bof, const std::vector<pe_import>& imports);
void bof_resolve_imports(binary_object_file& bof);

// in-memory linking

struct instance_section {
	char* memory;
	int pages;
};

struct instance {
	std::map<std::string, char*> symbols;
	std::vector<instance_section> sections;
};

void bof_save_mem(binary_object_file& bof, const std::vector<pe_import>& imports, instance& result);
