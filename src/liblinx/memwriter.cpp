#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cstddef>
#include <stdint.h>
#include "../graph/crt/include/sys/mman.h"
#include "bof.h"

void instance_section_add_symbols(codebits& code, std::vector<symbol>& symboltable, char* memory, std::map<std::string, char*>& symbols) {
	for (std::vector<symbol>::iterator i = symboltable.begin(); i != symboltable.end(); ++i) {
		if (i->section == code.name && i->section_index == code.index) {
			symbols[i->name] = memory + i->value;
		}
	}
}

void bof_save_mem(binary_object_file& bof, const std::vector<pe_import>& imports, instance& result) {

	for (std::vector<codebits>::iterator i = bof.code.begin(); i != bof.code.end(); ++i) {
		instance_section section;
		section.pages = (i->bytes.size() + 4095) / 4096;
		int codepage_size = 4096 * section.pages; 
		section.memory = (char*)mmap(NULL, codepage_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
		memcpy(section.memory, &i->bytes[0], i->bytes.size());

		result.sections.push_back(section);

		instance_section_add_symbols(*i, bof.symbols, section.memory, result.symbols);
	}
	
	for (std::vector<codebits>::iterator i = bof.code.begin(); i != bof.code.end(); ++i) {
		int section_index = std::distance(bof.code.begin(), i);
		instance_section& section = result.sections[section_index];
		
		for(std::vector<relocation>::iterator j = i->relocations.begin(); j != i->relocations.end(); ++j) {
			char* p = result.symbols[j->symbol];
			int32_t* dataptr = (int32_t*)&section.memory[j->offset];
			switch (j->type) {
				case r_32:
					*dataptr = (int32_t)p;
					break;
				case r_32_nobase:
					*dataptr = (int32_t)p;
					break;
				case r_pc32:
					// this is relative .... 32 bit relative offset between dataptr and p
					*dataptr += (p - (char*)dataptr);
					break;
				default:
					assert(false);
					break;
			}

		}
	}
}
