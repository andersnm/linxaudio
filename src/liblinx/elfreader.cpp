#include <iostream>
#include <libelf.h>
#include <gelf.h>
#if defined(_MSC_VER)
#include <io.h>
#define open _open
#define close _close
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include "bof.h"

struct elfrelocation {
	int type;
	int offset;
	int symbol;
};

struct elfsymbol {
	std::string name;
	int section;
	int value;
	int size;
	int info;
};

using std::cout;
using std::endl;

const char* get_section_header_name(int type) {
	switch(type) {
		case SHT_NULL: return ( "SHT_NULL");
		case SHT_PROGBITS: return ( "SHT_PROGBITS");
		case SHT_SYMTAB: return ( "SHT_SYMTAB");
		case SHT_STRTAB: return ( "SHT_STRTAB");
		case SHT_RELA: return ( "SHT_RELA");
		case SHT_HASH: return ( "SHT_HASH");
		case SHT_DYNAMIC: return ( "SHT_DYNAMIC");
		case SHT_NOTE: return ( "SHT_NOTE");
		case SHT_NOBITS: return ( "SHT_NOBITS");
		case SHT_REL: return ( "SHT_REL");
		case SHT_SHLIB: return ( "SHT_SHLIB");
		case SHT_DYNSYM: return ( "SHT_DYNSYM");
		case SHT_INIT_ARRAY: return ( "SHT_INIT_ARRAY");
		case SHT_FINI_ARRAY: return ( "SHT_FINI_ARRAY");
		case SHT_PREINIT_ARRAY: return ( "SHT_PREINIT_ARRAY"); 
		case SHT_GROUP: return ( "SHT_GROUP");
		case SHT_SYMTAB_SHNDX: return ( "SHT_SYMTAB_SHNDX"); 
		case SHT_NUM: return ( "SHT_NUM");
		case SHT_LOOS: return ( "SHT_LOOS");
		case SHT_GNU_verdef: return ( "SHT_GNU_verdef");
		case SHT_GNU_verneed: return ( "SHT_VERNEED");
		case SHT_GNU_versym: return ( "SHT_GNU_versym");
		default: return ( "(unknown) ");
	} 
}

const char* get_symbol_binding_name(int info) {
	switch(ELF32_ST_BIND(info)) {
		case STB_LOCAL: return ("LOCAL"); break;
		case STB_GLOBAL: return ("GLOBAL"); break;
		case STB_WEAK: return ("WEAK"); break;
		case STB_NUM: return ("NUM"); break;
		case STB_LOOS: return ("LOOS"); break;
		case STB_HIOS: return ("HIOS"); break;
		case STB_LOPROC: return ("LOPROC"); break;
		case STB_HIPROC: return ("HIPROC"); break;
		default: return ("UNKNOWN"); break;
	}
}

const char* get_symbol_type_name(int info) {
	switch(ELF32_ST_TYPE(info)) {
		case STT_NOTYPE: return ("NOTYPE"); break;
		case STT_OBJECT: return ("OBJECT"); break;
		case STT_FUNC:  return ("FUNC"); break;
		case STT_SECTION: return ("SECTION"); break;
		case STT_FILE: return ("FILE"); break;
		case STT_COMMON: return ("COMMON"); break;
		case STT_TLS: return ("TLS"); break;
		case STT_NUM: return ("NUM"); break;
		case STT_LOOS: return ("LOOS"); break;
		case STT_HIOS: return ("HIOS"); break;
		case STT_LOPROC: return ("LOPROC"); break;
		case STT_HIPROC: return ("HIPROC"); break;
		default: return ("UNKNOWN"); break;
	}
}

/*
enum RtT_Types {
	R_386_NONE		= 0, // No relocation
	R_386_32		= 1, // Symbol + Offset
	R_386_PC32		= 2  // Symbol + Offset - Section Offset
};
*/

#if !defined(R_386_NONE)
#define R_386_NONE 0
#endif

#if !defined(R_386_32)
#define R_386_32 1
#endif

#if !defined(R_386_PC32)
#define R_386_PC32 2
#endif

const char* get_reltype_name(int info) {
	switch (ELF32_R_TYPE(info)) {
		case R_386_NONE:
			return "R_386_NONE";
		case R_386_32:
			return "R_386_32";
		case R_386_PC32:
			return "R_386_PC32";
		default:
			return "unknown";
	}
}
bool parse_rel(Elf *e, const GElf_Shdr& shdr, Elf_Scn *scn, const std::string& name, std::vector<elfrelocation>& result) {
	Elf_Data* edata = 0;
	edata = elf_getdata(scn, edata);
	int rel_count = shdr.sh_size / shdr.sh_entsize;
	for (int i = 0; i < rel_count; i++) {
		// TODO: header says ELFCLASS32 / ELFCLASS64 -> use ELF64_xxx or ELF32_xxx
		GElf_Rel rel;
		gelf_getrel(edata, i, &rel);

		elfrelocation r;
		r.offset = rel.r_offset;		
		r.symbol = ELF64_R_SYM(rel.r_info);
		r.type = ELF64_R_TYPE(rel.r_info);
		result.push_back(r);

		cout << "  Rel: " << get_reltype_name(rel.r_info)<<  ", " << r.symbol << ", offset: " << rel.r_offset << endl;
	}
	return true;
}

void parse_symtab(Elf *e, const GElf_Shdr& shdr, Elf_Scn *scn, const std::string& name, std::vector<elfsymbol>& result) {
	Elf_Data* edata = 0;
	edata = elf_getdata(scn, edata);
	int symbol_count = shdr.sh_size / shdr.sh_entsize;

	for (int i = 0; i < symbol_count; i++) {
		GElf_Sym sym;
		gelf_getsym(edata, i, &sym);
		
		// print out the value and size
		cout << "  Value: " << sym.st_value << ", size: " << sym.st_size << ", " << 
			get_symbol_binding_name(sym.st_info) << ", " << get_symbol_type_name(sym.st_info) << ", " <<
			elf_strptr(e, shdr.sh_link, sym.st_name) << ", section " << sym.st_shndx << endl;
		
		elfsymbol s;
		s.name = elf_strptr(e, shdr.sh_link, sym.st_name);
		s.value = sym.st_value;
		s.size = sym.st_size;
		s.section = sym.st_shndx;
		s.info = sym.st_info;
		result.push_back(s);
	}
}

void parse_codebits(Elf *e, const GElf_Shdr& shdr, Elf_Scn *scn, const std::string& name, int index, std::vector<codebits>& result) {
	if (shdr.sh_size == 0 || name == ".comment") {
		return ;
	}

	Elf_Data* edata = 0;
	edata = elf_getdata(scn, edata);
	
	char* data = (char*)edata->d_buf;
	codebits code;
	code.name = name;
	code.index = 0; // this is OK if elf section names are unique! not verified!
	code.bytes.assign(data, data + shdr.sh_size);
	result.push_back(code);
}

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

bool open_elf(const std::string& filename, int* resultf, Elf** result, GElf_Ehdr* resulthdr) {
	int f = open(filename.c_str(), O_RDONLY|O_BINARY, 0);
	if (f < 0) {
		return false;
	}

	Elf *e = elf_begin(f, ELF_C_READ, NULL);
	if (e == 0) {
		close(f);
		return false;
	}

	if (elf_kind(e) != ELF_K_ELF) {
		elf_end(e);
		close(f);
		return false;
	}

	GElf_Ehdr ehdr_in;
	gelf_getehdr(e, &ehdr_in);

	if (ehdr_in.e_type != ET_REL) {
		elf_end(e);
		close(f);
		return false;
	}
	
	//ELFOSABI_NONE ;
	//ehdr_in.e_ident[EI_OSABI] != ELFOSABI_LINUX ;

	switch (ehdr_in.e_ident[EI_DATA] ) {
		case ELFDATA2LSB:
			//target_is_big_endian = 0;
			break;
		case ELFDATA2MSB:
			//target_is_big_endian = 1;
			break;
	}

	*resultf = f;
	*result = e;
	*resulthdr = ehdr_in;

	return true;
}

void process_symtab(std::map<int, std::string>& sections, std::vector<elfsymbol>& symbols, std::vector<symbol>& result) {
	for (std::vector<elfsymbol>::iterator i = symbols.begin(); i != symbols.end(); ++i) {
		int binding = ELF32_ST_BIND(i->info);
		int elftype = ELF32_ST_TYPE(i->info);

		symboltype type;
		std::string section;
		std::string name;
		switch (elftype) {
			case STT_NOTYPE:
				// NOTE: first symbol is always a notype name="" size=0
				if (i->name.empty() && i->size == 0 && i->section == 0 && i->value == 0) {
					continue;
				} else {
					type = symbol_none;
					name = i->name;
				}
				break;
			case STT_FILE: 
				continue;
			case STT_SECTION:
				type = symbol_section;
				name = sections[i->section];
				section = sections[i->section];
				// elf section symbols are blank, fixup to section name so it can be looked up by its name:
				i->name = sections[i->section];
				break;
			case STT_FUNC: 
				type = symbol_func; 
				name = i->name;
				section = sections[i->section];
				break;
			case STT_OBJECT: 
				type = symbol_object; 
				name = i->name;
				section = sections[i->section];
				break;
			default: assert(false); continue;
		}

		symbol s;
		s.name = name;
		s.section = section;
		s.section_index = 0;
		s.type = type;
		s.value = i->value;
		s.size = i->size;
		s.is_global = (binding == STB_GLOBAL);
		s.is_export = false;
		result.push_back(s);
	}
}

bool process_relocations(binary_object_file& result, std::vector<elfsymbol>& symbols, const std::string& name, std::vector<elfrelocation>& relocations) {

	// TODO: not sure if this is the right way to get the relocations for this section
	std::string::size_type relpos = name.find(".rel");
	if (relpos != 0) {
		// relocation section should be prefixed with ".rel"
		return false;
	}
	std::string codename = name.substr(4);
	codebits* code = bof_find_code(result, codename, 0);
	if (code == 0) {
		// cant find code section for relocations = bad
		return false;
	}

	for (std::vector<elfrelocation>::iterator j = relocations.begin(); j != relocations.end(); ++j) {
		elfsymbol& sym = symbols[j->symbol];
		relocation r;
		r.offset = j->offset;
		r.symbol = sym.name;
		switch (j->type) {
			case R_386_32: r.type = r_32; break;
			case R_386_PC32: r.type = r_pc32; break;
			default: return false;
		}
		code->relocations.push_back(r);
	}
	return true;
}

#if !defined(EM_AMD64)
#define EM_AMD64 62
#endif

bool bof_load_elf(const std::string& filename, binary_object_file& result) {
	if (elf_version(EV_CURRENT) == EV_NONE) {
		return false;
	}

	Elf* e = 0;
	int fd_in;
	GElf_Ehdr ehdr_in;

	if (!open_elf(filename, &fd_in, &e, &ehdr_in)) {
		return false;
	}

	switch (ehdr_in.e_machine) {
		case EM_386:
			result.arch = arch_i386;
			break;
		case EM_AMD64:
			result.arch = arch_x64;
			break;
		case EM_ARM:
			//result.arch = arch_arm;
			//EF_ARM_ABIMASK == 0x05000000 for abi v5
			//break;
		default:
			elf_end(e);
			close(fd_in);
			return false;
	}

	size_t shstrndx;
	if (elf_getshdrstrndx(e, &shstrndx) != 0) {
		elf_end(e);
		close(fd_in);
		return false;
	}

	// first pass: parse elf contents into intermediate structs
	std::map<int, std::string> sections;
	std::map<std::string, std::vector<elfrelocation> > relocations;
	std::vector<elfsymbol> symbols;

	Elf_Scn *scn = NULL;
	GElf_Shdr shdr;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		gelf_getshdr(scn, &shdr);
 
		int index = elf_ndxscn(scn);
		const char* name = elf_strptr(e, shstrndx, shdr.sh_name);
 
		cout << "Section " << index << ": " << name << " " << get_section_header_name(shdr.sh_type) << ", " << shdr.sh_size << " bytes" << endl;

		sections.insert(std::pair<int, std::string>(index, name));

		if (shdr.sh_type == SHT_SYMTAB) {
			parse_symtab(e, shdr, scn, name, symbols);
		} else if (shdr.sh_type == SHT_REL) {
			parse_rel(e, shdr, scn, name, relocations[name]);
		} else if (shdr.sh_type == SHT_PROGBITS) {
			parse_codebits(e, shdr, scn, name, index, result.code);
		}
	}

	elf_end(e);
	close(fd_in);

	// second pass: expand section and symbol indices into names used by the bof structs

	process_symtab(sections, symbols, result.symbols);

	for (std::map<std::string, std::vector<elfrelocation> >::iterator i = relocations.begin(); i != relocations.end(); ++i) {
		process_relocations(result, symbols, i->first, i->second);
	}

	return true;
}



/*
void dump_symtab(Elf *e, const GElf_Shdr& shdr, Elf_Scn *scn) {
	Elf_Data* edata = 0;
	edata = elf_getdata(scn, edata);
	int symbol_count = shdr.sh_size / shdr.sh_entsize;

	// loop through to grab all symbols
	for (int i = 0; i < symbol_count; i++) {
		GElf_Sym sym;
		gelf_getsym(edata, i, &sym);

		// print out the value and size
		cout << "  Value: " << sym.st_value << ", size: " << sym.st_size << ", " << 
			get_symbol_binding_name(sym.st_info) << ", " << get_symbol_type_name(sym.st_info) << ", " <<
			elf_strptr(e, shdr.sh_link, sym.st_name) << ", section " << sym.st_shndx << endl;
	}
}
*/

/*
void dump_rel(Elf *e, const GElf_Shdr& shdr, Elf_Scn *scn) {
	Elf_Data* edata = 0;
	edata = elf_getdata(scn, edata);
	int rel_count = shdr.sh_size / shdr.sh_entsize;
	for (int i = 0; i < rel_count; i++) {
		GElf_Rel rel;
		gelf_getrel(edata, i, &rel);
		cout << "  Rel: " << get_reltype_name(rel.r_info)<<  ", " << ELF32_R_SYM(rel.r_info) << ", offset: " << rel.r_offset << endl;
		//elf_get_symval()
	}
}

static void dump_sections_name(Elf *e) {
	Elf_Scn *scn=NULL;
	GElf_Shdr shdr;
	size_t n, shstrndx, sz;
 
	if (elf_getshdrstrndx(e, &shstrndx) != 0) {
		//errx(EX_SOFTWARE, "elf_getshdrstrndx() failed : %s . " , elf_errmsg(-1));
	}
 
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		gelf_getshdr(scn, &shdr);
 
		const char* name = elf_strptr(e, shstrndx, shdr.sh_name);
 
		cout << "Section " << (int) elf_ndxscn(scn) << ": " << name << " " << get_section_header_name(shdr.sh_type) << ", " << shdr.sh_size << " bytes" << endl;

		if (shdr.sh_type == SHT_SYMTAB) {
			dump_symtab(e, shdr, scn);
		} else if (shdr.sh_type == SHT_REL) {
			dump_rel(e, shdr, scn);
		}

	}
}
*/
