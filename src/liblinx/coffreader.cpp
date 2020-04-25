/*
References:

	http://msdn.microsoft.com/en-us/gg463119.aspx (docx)

	http://wiki.osdev.org/COFF

*/
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <fstream>
#include <cstring>
#include <pe_bliss.h>
#include "bof.h"

#pragma pack (push, 1)
struct coff_relocation_entry {
	uint32_t VirtualAddress;
	uint32_t SymbolTableIndex;
	uint16_t Type;
};
#pragma pack (pop)

std::string zpadstring(uint8_t* str, int len) {
	// An 8-byte, null-padded UTF-8 encoded string. If the string is exactly 8 characters long, 
	// there is no terminating null. For longer names, this field contains a slash (/) that is 
	// followed by an ASCII representation of a decimal number that is an offset into the string table.
	if (str[len - 1] != 0) {
		return std::string(str, str + len);
	} else {
		return std::string(str, str + strlen((const char*)str));
	}
}

std::string get_stringtab_string(std::vector<char>& result, int offset) {
	int length = strlen(&result[offset]);
	return std::string(result.begin() + offset, result.begin() + offset + length);
}

std::string get_symbol_name(pe_bliss::pe_win::image_symbol& symbol, std::vector<char>& stringtab) {
	std::string name;
	if (symbol.N.Name.Short == 0) {
		// first four bytes are 0, Name.Long is an offset into the string table
		// this means stringtab is bogus ... need rawbytes to account for clever packing ... probably not required
		name = get_stringtab_string(stringtab, symbol.N.Name.Long - 4);
	} else {
		name = zpadstring(symbol.N.ShortName, 8);
	}

	// TODO: quickhack to link object files from both gcc and vc - ??
	if (name[0] == '_') {
		return name.substr(1);
	} else {
		return name;
	}
}


const int image_file_machine_i386 = 0x14c;
const int image_file_machine_amd64 = 0x8664;
const int image_file_machine_arm = 0x1c0;
const int image_file_machine_armv7 = 0x1c4;

const uint32_t image_rel_i386_dir32 = 0x06;
const uint32_t image_rel_i386_dir32nb = 0x07;
const uint32_t image_rel_i386_section = 0x0a;
const uint32_t image_rel_i386_secrel = 0x0b;
const uint32_t image_rel_i386_rel32 = 0x14;

const uint32_t image_rel_amd64_addr64 = 0x01;
const uint32_t image_rel_amd64_addr32nb = 0x03;
const uint32_t image_rel_amd64_rel = 0x04;
const uint32_t image_rel_amd64_rel32_1 = 0x05;

/*
 IMAGE_REL_AMD64_ABSOLUTE = 0x0000, IMAGE_REL_AMD64_ADDR64 = 0x0001, IMAGE_REL_AMD64_ADDR32 = 0x0002, IMAGE_REL_AMD64_ADDR32NB = 0x0003,
  IMAGE_REL_AMD64_REL32 = 0x0004, IMAGE_REL_AMD64_REL32_1 = 0x0005, IMAGE_REL_AMD64_REL32_2 = 0x0006, IMAGE_REL_AMD64_REL32_3 = 0x0007,
  IMAGE_REL_AMD64_REL32_4 = 0x0008, IMAGE_REL_AMD64_REL32_5 = 0x0009, IMAGE_REL_AMD64_SECTION = 0x000A, IMAGE_REL_AMD64_SECREL = 0x000B,
  IMAGE_REL_AMD64_SECREL7 = 0x000C, IMAGE_REL_AMD64_TOKEN = 0x000D, IMAGE_REL_AMD64_SREL32 = 0x000E, IMAGE_REL_AMD64_PAIR = 0x000F,
  IMAGE_REL_AMD64_SSPAN32 = 0x0010 */

void parse_section(std::istream& f, int machine, pe_bliss::pe_win::image_section_header& section, int index, std::vector<pe_bliss::pe_win::image_symbol>& symbols, std::vector<char>& stringtab, std::vector<codebits>& result) {
	f.seekg(section.PointerToRawData);

	if ((section.Characteristics & pe_bliss::pe_win::image_scn_cnt_code) ||
	    (section.Characteristics & pe_bliss::pe_win::image_scn_cnt_initialized_data) ||
	    (section.Characteristics & pe_bliss::pe_win::image_scn_cnt_uninitialized_data))
	{

		bool is_discardable = (section.Characteristics & pe_bliss::pe_win::image_scn_mem_discardable);
		if (is_discardable) return; // usually debug stuff

		codebits code;
		code.name = zpadstring(section.Name, 8);
		code.index = index;

		code.bytes.resize(section.SizeOfRawData);
		if ((section.Characteristics & pe_bliss::pe_win::image_scn_cnt_uninitialized_data) ) {
			code.is_uninitialized = true;
		} else {
			if (section.SizeOfRawData > 0) {
				f.read(&code.bytes[0], section.SizeOfRawData);
			}
		}
			
		if (section.Characteristics & pe_bliss::pe_win::image_scn_mem_execute) {
			code.is_executable = true;
		} 
		if (section.Characteristics & pe_bliss::pe_win::image_scn_mem_read) {
			code.is_readable = true;
		}
		if (section.Characteristics & pe_bliss::pe_win::image_scn_mem_write) {
			code.is_writable = true;
		}

		// parse section relocations
		if (section.NumberOfRelocations > 0) {
			f.seekg(section.PointerToRelocations);
			for (int i = 0; i < section.NumberOfRelocations; i++) {
				coff_relocation_entry relo;
				f.read(reinterpret_cast<char*>(&relo), sizeof(coff_relocation_entry));
				
				// need symbol table here, or need to keep relocation for later later
				relocation r;
				pe_bliss::pe_win::image_symbol& symbol = symbols[relo.SymbolTableIndex];
				r.symbol = get_symbol_name(symbol, stringtab);
				r.offset = relo.VirtualAddress;
				switch (machine) {
					case image_file_machine_i386:
						switch (relo.Type) {
							case image_rel_i386_rel32:
								r.type = r_pc32;
								break;
							case image_rel_i386_dir32:
								r.type = r_32;
								break;
							case image_rel_i386_dir32nb:
								r.type = r_32_nobase;
								break;
							case image_rel_i386_secrel:
								r.type = r_32_secrel;
								break;
							case image_rel_i386_section:
								r.type = r_32_section;
								break;
							default:
								assert(false);
								break;
						}
						break;
					case image_file_machine_amd64:
						switch (relo.Type) {
							case image_rel_amd64_addr64:
								r.type = r_64_addr64;
								break;
							case image_rel_amd64_addr32nb:
								r.type = r_64_addr32nb;
								break;
							case image_rel_amd64_rel:
								r.type = r_64_rel;
								break;
							case image_rel_amd64_rel32_1:
								r.type = r_64_rel32_1;
								break;
							default: 
								assert(false);
								break;
						}
						break;
					default:
						assert(false); // machine not implemented
						break;
				}
				code.relocations.push_back(r);
			}
		}

		result.push_back(code);
	} else {
		int nox = 0;
	}
}

void parse_stringtab(std::istream& f, pe_bliss::pe_win::image_file_header& header, std::vector<char>& result) {
	f.seekg(header.PointerToSymbolTable + header.NumberOfSymbols * sizeof(pe_bliss::pe_win::image_symbol));
	unsigned int stringtabsize;
	f.read(reinterpret_cast<char*>(&stringtabsize), sizeof(stringtabsize));
	result.resize(stringtabsize - 4);
	if (!result.empty()) {
		f.read(&result[0], result.size());
	}
}

bool bof_load_coff(const std::string& filename, binary_object_file& result) {

	std::ifstream f(filename.c_str(), std::ios::binary | std::ios::in);
	if (!f) {
		return false;
	}
	pe_bliss::pe_win::image_file_header header;
	f.read(reinterpret_cast<char*>(&header), sizeof(pe_bliss::pe_win::image_file_header));

	switch (header.Machine) { 
		case image_file_machine_i386:
			result.arch = arch_i386;
			break;
		case image_file_machine_amd64:
			result.arch = arch_x64;
			break;
		case image_file_machine_arm:
			result.arch = arch_arm;
			break;
		case image_file_machine_armv7:
			result.arch = arch_armv7;
			break;
		default:
			return false;
	}
	

	std::vector<char> stringtab;
	parse_stringtab(f, header, stringtab);

	std::vector<pe_bliss::pe_win::image_symbol> symbols;
	f.seekg(header.PointerToSymbolTable);
	for (int i = 0; i < header.NumberOfSymbols; i++) {
		pe_bliss::pe_win::image_symbol symbol;
		f.read(reinterpret_cast<char*>(&symbol), sizeof(pe_bliss::pe_win::image_symbol));
		symbols.push_back(symbol);
	}

	f.seekg(sizeof(pe_bliss::pe_win::image_file_header) + header.SizeOfOptionalHeader);
	
	std::vector<pe_bliss::pe_win::image_section_header> sections;
	for (int i = 0; i < header.NumberOfSections; i++) {
		pe_bliss::pe_win::image_section_header section;
		f.read(reinterpret_cast<char*>(&section), sizeof(pe_bliss::pe_win::image_section_header));
		sections.push_back(section);
	}

	for (std::vector<pe_bliss::pe_win::image_section_header>::iterator i = sections.begin(); i != sections.end(); ++i) {
		parse_section(f, header.Machine, *i, (int)std::distance(sections.begin(), i), symbols, stringtab, result.code);
	}

	int auxcount = 0;
	for (std::vector<pe_bliss::pe_win::image_symbol>::iterator i = symbols.begin(); i != symbols.end(); ++i) {
		pe_bliss::pe_win::image_symbol& symbol = *i;

		if (auxcount > 0) {
			auxcount--;
		} else {
			// is a primary symbol, non-aux
			int symclass = symbol.StorageClass;
			int symtype = symbol.Type >> pe_bliss::pe_win::n_btshft;
			bool is_global = false;
			switch (symbol.StorageClass) {
				case pe_bliss::pe_win::image_sym_class_external:
					is_global = true;
					break;
				case pe_bliss::pe_win::image_sym_class_static:
					is_global = false;
					break;
				case pe_bliss::pe_win::image_sym_class_label:
					is_global = false;
					break;
				/*case pe_bliss::pe_win::image_sym_class_function:
					break;
				case pe_bliss::pe_win::image_sym_class_block:
					break;
				case pe_bliss::pe_win::image_sym_class_section:
					break;*/
				default:
					assert(0);
					false;
			}
			

			std::string name = get_symbol_name(symbol, stringtab);

			if (symtype == pe_bliss::pe_win::image_sym_dtype_function) {
				::symbol sym;
				sym.name = name;
				sym.size =  symbol.Value;
				sym.is_global = is_global;
				sym.is_export = false;
				if (symbol.SectionNumber == 0) {
					// unresolved - not defined here
					sym.type = symbol_none;
					sym.value = 0;
				} else {
					// defined here
					sym.type = symbol_func;
					sym.section = zpadstring(sections[symbol.SectionNumber - 1].Name, 8);
					sym.section_index = symbol.SectionNumber - 1;
					sym.value = symbol.Value;
				}
				result.symbols.push_back(sym);
			} else if (symtype == pe_bliss::pe_win::image_sym_dtype_null && symbol.SectionNumber >= 0 /*&& (symbol.StorageClass == 2 || symbol.StorageClass == 3)*/) {
				// storageclass 2 == external, 3 == static
				::symbol sym;
				sym.name = name;
				sym.size =  0;
				sym.is_global = is_global;
				sym.is_export = false;
				if (symbol.SectionNumber == 0) {
					// unresolved - not defined here
					sym.type = symbol_none;
					sym.value = 0;
				} else {
					sym.section = zpadstring(sections[symbol.SectionNumber - 1].Name, 8);
					sym.section_index = symbol.SectionNumber - 1;
					sym.is_global = is_global;
					if (sym.section == name) {
						sym.type = symbol_section;
						//sym.is_global = false;
						sym.value = 0;
					} else {
						sym.type = symbol_object;
						//sym.is_global = true;
						sym.value = symbol.Value;
					}
				}
				result.symbols.push_back(sym);
			} else {
				// skipping something ... not sure what
				//std::cout << "skipping " << name << std::endl;
			}
		}
		auxcount += symbol.NumberOfAuxSymbols;
	}

	return true;
}
