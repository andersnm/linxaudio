#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cstddef>
#include <pe_bliss.h>
#include "bof.h"

const int pe_rvastart = 0x1000; // header is 0x1000

void bof_build_pe_relocs(binary_object_file& bof, codebits& code, int alignment, int rva, std::string& data, pe_bliss::pe_base& p, pe_bliss::relocation_table_list& tables) {

	// NOTE: each table contains fixups per 4k page of data
	// ie, cant just say "relocate offset 5100", need to create a new 
	// table pointing at 4096, and then relocate offset (5100-4096) in that
	// http://geezer.osdevbrasil.net/osd/exec/pe.txt

	// 1. get max relocation offset
	// 2. divide by page size to get number of tables
	// 3. loop that and apply ranges
	int max_relo = bof_find_max_relocation_offset(code);
	int relo_tables = (max_relo / alignment) + 1;

	for (int i = 0; i < relo_tables; i++) {

		int chunk_start = i * alignment;
		pe_bliss::relocation_table new_table;
		new_table.set_rva(rva + chunk_start); 

		for (std::vector<relocation>::iterator j = code.relocations.begin(); j != code.relocations.end(); ++j) {
			symbol* relosym = bof_find_symbol(bof, j->symbol);
			assert(relosym != 0);

			if (j->offset < chunk_start || j->offset >= chunk_start + alignment) {
				continue;
			}

			symbol* relosecsym = bof_find_symbol(bof, relosym->section);
			int sectionstart = relosecsym->value;

			int32_t* dataptr = (int32_t*)(&data[j->offset]);

			if (j->type == r_pc32) {
				// .. patch relative relocations directly
				*dataptr =  relosym->value - j->offset - 4;
				// used to do this: ???
				//*dataptr =  sectionstart + relosym->value - j->offset - 4;
			} else if (j->type == r_32) {
				// 3 = IMAGE_REL_BASED_HIGHLOW

				// NOTE: the compiler could place an offset into the symbol address here
				*dataptr += p.get_image_base_32() + pe_rvastart + sectionstart + relosym->value;
				if ((j->offset - chunk_start ) >= (alignment - (int)sizeof(int))) {
					std::cout << "WARNING: bad relocation for symbol '" << relosym->name << "'" << std::endl;
				}
				new_table.add_relocation(pe_bliss::relocation_entry(j->offset - chunk_start, 3));
			} else if (j->type = r_32_nobase) {
				// same as r_32 but without image base, used internally to patch import data
				*dataptr = pe_rvastart + sectionstart + relosym->value;
			} else {
				assert(0);
			}
		}
		if (!new_table.get_relocations().empty()) {
			tables.push_back(new_table);
		}
	}
}

bool bof_save_pe_inner(binary_object_file& bof, const std::string& filename) {
	/*pe_bliss::pe_properties& props;
	if (bof.arch == arch_i386) {
		props = pe_bliss::pe_properties_32();
	} else {
	}*/
	
	pe_bliss::pe_base p(pe_bliss::pe_properties_32(), 4096, true, pe_bliss::pe_win::image_subsystem_windows_gui); // 2 = IMAGE_SUBSYSTEM_WINDOWS_GUI
	p.set_machine(0x14c); // 0x14c = IMAGE_FILE_MACHINE_I386
	p.set_characteristics(pe_bliss::pe_win::image_file_executable_image | pe_bliss::pe_win::image_file_32bit_machine | pe_bliss::pe_win::image_file_dll | pe_bliss::pe_win::image_file_debug_stripped );
	p.set_dll_characteristics(pe_bliss::pe_win::image_dllcharacteristics_dynamic_base);
	p.set_image_base(0x10000000);

	std::ofstream new_pe_file(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!new_pe_file) {
		return false;
	}


	int ordinal = 0;
	pe_bliss::relocation_table_list tables;
	pe_bliss::exported_functions_list exports;

	// first calculate section offsets and update the section symbol
	int codeoffset = 0;
	for (std::vector<codebits>::iterator i = bof.code.begin(); i != bof.code.end(); ++i) {
		symbol* secsym = bof_find_symbol(bof, i->name);
		secsym->value = codeoffset;

		uint32_t alignment = p.get_section_alignment();
		int pages = (i->bytes.size() - 1) / alignment;
		codeoffset += (pages + 1) * alignment;
	}

	for (std::vector<codebits>::iterator i = bof.code.begin(); i != bof.code.end(); ++i) {
		symbol* secsym = bof_find_symbol(bof, i->name);
		int rva = pe_rvastart + secsym->value;

		assert(i->bytes.size() > 0);
		pe_bliss::section s;
		s.set_name(i->name);
		s.executable(true).readable(true).writeable(true);
		//s.executable(i->is_executable).readable(i->is_readable).writeable(i->is_writable);
		s.set_virtual_address(rva); // same as rva below

		std::string data(i->bytes.begin(), i->bytes.end()); 
		bof_build_pe_relocs(bof, *i, 4096, rva, data, p, tables);
		s.set_raw_data(data);
		p.add_section(s);

		// add exports in this section
		for (std::vector<symbol>::iterator j = bof.symbols.begin(); j != bof.symbols.end(); ++j) {
			if (j->section != i->name || j->type != symbol_func || !j->is_export) {
				continue;
			}
			pe_bliss::exported_function ef;
			ef.set_name(j->name);
			ef.set_rva(rva + j->value);
			ef.set_ordinal(ordinal);
			exports.push_back(ef);
			ordinal ++;
		}
	}

	{
		pe_bliss::section new_relocs;
		new_relocs.get_raw_data().resize(1);
		new_relocs.set_name(".reloc");
		new_relocs.readable(true);
		pe_bliss::section& attached_section = p.add_section(new_relocs);
		pe_bliss::rebuild_relocations(p, tables, attached_section);
	}

	{
		pe_bliss::section new_exports;
		new_exports.get_raw_data().resize(1);
		new_exports.set_name(".edata");
		new_exports.readable(true);
		pe_bliss::section& attached_section = p.add_section(new_exports);

		pe_bliss::export_info info;
		info.set_name(filename);
		info.set_ordinal_base(0);
		pe_bliss::rebuild_exports(p, info, exports, attached_section);
	}

	codebits* idata = bof_find_code(bof, ".idata", 0);
	symbol* idatasym = bof_find_symbol(bof, ".idata");
	assert(idata && idatasym);

	p.set_directory_rva(pe_bliss::pe_win::image_directory_entry_import, pe_rvastart + idatasym->value);
	p.set_directory_size(pe_bliss::pe_win::image_directory_entry_import, idata->bytes.size());

	{
		std::stringstream temp_pe(std::ios::out | std::ios::in | std::ios::binary);
		pe_bliss::rebuild_pe(p, temp_pe);
		p.set_checksum(pe_bliss::calculate_checksum(temp_pe));
	}
	//p.set_stub_overlay("liblinx");
	pe_bliss::rebuild_pe(p, new_pe_file);
	new_pe_file.close();
	return true;
}

void bof_get_distinct_imports(binary_object_file& bof, std::vector<std::string>& result) {
	std::set<std::string> ss;
	for (std::vector<symbol>::iterator i = bof.symbols.begin(); i != bof.symbols.end(); ++i) {

		if (i->import_module.empty()) {
			continue;
		}
		if (std::find(result.begin(), result.end(), i->import_module) == result.end()) {
			result.push_back(i->import_module);
		}
	}
}

void bof_get_import_symbols(binary_object_file& bof, const std::string& import_name, std::vector<std::string>& result) {
	for (std::vector<symbol>::iterator i = bof.symbols.begin(); i != bof.symbols.end(); ++i) {

		if (i->import_module != import_name) {
			continue;
		}

		result.push_back(i->name);
	}
}

void bof_graph_add_relocation(codebits& code, relocationtype type, const std::string& symbol, int offset) {
	relocation namerelo;
	namerelo.type = type;
	namerelo.symbol = symbol;
	namerelo.offset = offset;
	code.relocations.push_back(namerelo);

}

void bof_graph_add_symbol(binary_object_file& bof, bool is_global, const std::string& name, const std::string& section, int section_index, symboltype type, int value, int size, const std::string& import_module, const std::string& import_mangled) {
	symbol namesym;
	namesym.is_global = is_global;
	namesym.is_export = false;
	namesym.name = name;
	namesym.section = section;
	namesym.section_index = section_index;
	namesym.type = type;
	namesym.value = value;
	namesym.size = size;
	namesym.import_module = import_module;
	namesym.import_mangled = import_mangled;
	bof.symbols.push_back(namesym);
}

void bof_resolve_imports(binary_object_file& bof) {
	
	std::vector<std::string> imports;
	bof_get_distinct_imports(bof, imports);

	int total_import_count = 0;
	int size_of_strings = 0;

	for (std::vector<std::string>::iterator j = imports.begin(); j != imports.end(); ++j) {

		size_of_strings += j->length() + 1; // +1 for terminating zero char
		std::vector<std::string> import_symbols;
		bof_get_import_symbols(bof, *j, import_symbols);
		for (std::vector<std::string>::iterator i = import_symbols.begin(); i != import_symbols.end(); ++i) {
			size_of_strings += i->length() + 1 + sizeof(uint16_t); // +1 for terminating zero char + hint
			total_import_count++;
		}
	}

	// +1 for terminating zero descriptor
	int size_of_descriptors = (1 + total_import_count) * sizeof(pe_bliss::pe_win::image_import_descriptor);
	// the iat table is terminated by a null for each import
	int size_of_iat = (total_import_count + imports.size()) * 4;

	codebits code;
	code.is_readable = true;
	code.is_writable = true;
	code.is_executable = true;
	code.bytes.resize(size_of_strings + size_of_iat + size_of_descriptors);
	code.name = ".idata";
	code.index = 0;

	int current_descriptor_pos = 0;
	int current_string_pos = size_of_descriptors;
	int current_iat_pos = size_of_strings + size_of_descriptors;

	// write import dll names, function names and descriptors

	for (std::vector<std::string>::iterator j = imports.begin(); j != imports.end(); ++j) {

		int import_name_rva = current_string_pos;
		strcpy(&code.bytes[current_string_pos], j->c_str());
		pe_bliss::pe_win::image_import_descriptor iid = {0};
		iid.Name = current_string_pos;
		iid.FirstThunk = current_iat_pos;
		iid.OriginalFirstThunk = current_iat_pos;
		iid.ForwarderChain = -1;

		// the import descriptor fields must be relocated
		bof_graph_add_relocation(code, r_32_nobase, "$Name." + *j, current_descriptor_pos + offsetof(pe_bliss::pe_win::image_import_descriptor, Name));
		bof_graph_add_symbol(bof, false, "$Name." + *j, ".idata", 0, symbol_object, current_string_pos, 0);
		bof_graph_add_relocation(code, r_32_nobase, "$FirstThunk." + *j, current_descriptor_pos + offsetof(pe_bliss::pe_win::image_import_descriptor, FirstThunk));
		bof_graph_add_symbol(bof, false, "$FirstThunk." + *j, ".idata", 0, symbol_object, current_iat_pos, 0);
		bof_graph_add_relocation(code, r_32_nobase, "$OriginalFirstThunk." + *j, current_descriptor_pos + offsetof(pe_bliss::pe_win::image_import_descriptor, OriginalFirstThunk));
		bof_graph_add_symbol(bof, false, "$OriginalFirstThunk." + *j, ".idata", 0, symbol_object, current_iat_pos, 0);

		memcpy(&code.bytes[current_descriptor_pos], &iid, sizeof(pe_bliss::pe_win::image_import_descriptor));

		current_string_pos += j->length() + 1;
		current_descriptor_pos += sizeof(pe_bliss::pe_win::image_import_descriptor);

		std::vector<std::string> import_symbols;
		bof_get_import_symbols(bof, *j, import_symbols);
		for (std::vector<std::string>::iterator i = import_symbols.begin(); i != import_symbols.end(); ++i) {

			symbol* sym = bof_find_symbol(bof, *i);
			assert(sym != 0 && !sym->import_mangled.empty());

			symbol* impsym = bof_find_symbol(bof, sym->import_mangled);
			assert(impsym != 0);

			// NOTE: supports named imports only
			unsigned short hint = 0;
			memcpy(&code.bytes[current_string_pos], &hint, sizeof(uint16_t));
			strcpy(&code.bytes[current_string_pos + sizeof(uint16_t)], sym->name.c_str());
			memcpy(&code.bytes[current_iat_pos], &current_string_pos, 4);

			// update symbols as resolved
			impsym->section = ".idata";
			impsym->type = symbol_object;
			impsym->value = current_iat_pos;

			// copy the length now, because the sym pointer becomes invalid after add_symbol()!
			int symnamelength = sym->name.length();

			// the iat must be relocated
			bof_graph_add_relocation(code, r_32_nobase, "$HintName." + sym->name, current_iat_pos);
			bof_graph_add_symbol(bof, false, "$HintName." + sym->name, ".idata", 0, symbol_object, current_string_pos, 0);

			current_string_pos += symnamelength + 1 + sizeof(uint16_t);
			current_iat_pos += 4;
		}
		// leave a blank entry in the iat to indicate end-of-import
		current_iat_pos += 4;

	}

	bof.code.push_back(code);

	// add a symbol for the section
	bof_graph_add_symbol(bof, false, ".idata", ".idata", 0, symbol_section, 0, 0);
}

void bof_add_imports(binary_object_file& bof, const std::vector<pe_import>& imports) {
	std::vector<std::string> undefined_symbols;
	bof_get_undefined_symbols(bof, undefined_symbols);
	
	if (!undefined_symbols.empty()) {
		for (std::vector<std::string>::iterator i = undefined_symbols.begin(); i != undefined_symbols.end(); ++i) {
			const pe_import* imp = bof_find_import(imports, *i);

			if (imp != 0) {
				bof_graph_add_symbol(bof, false, imp->import_symbol, "", 0, symbol_func, 0, 0, imp->module_name, *i);
			}
		}
	}
}

bool bof_save_pe(const binary_object_file& bof, const std::string& filename, const std::vector<pe_import>& imports) {
	// copy the bof contents to a new, temporary bof and prepare for saving:
	// - pe relocations have slightly different semantics from elf and must be
	// patched in the databytes. 
	// - dll imports are specific to pe and are compiled into its own section
	// with symbols, relocations and data
	binary_object_file newf = bof;
	bof_add_imports(newf, imports);
	bof_resolve_imports(newf);
	return bof_save_pe_inner(newf, filename);
}
