#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <sstream>
#include "bof.h"

// if index == -1, returns first matching codebits
codebits* bof_find_code(binary_object_file& bof, const std::string& name, int index) {
	for (std::vector<codebits>::iterator i = bof.code.begin(); i != bof.code.end(); ++i) {
		if (i->name == name && (index == -1 || i->index == index)) return &*i;
	}
	return 0;
}

symbol* bof_find_symbol(binary_object_file& bof, const std::string& name) {
	for (std::vector<symbol>::iterator i = bof.symbols.begin(); i != bof.symbols.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

relocation* bof_find_relocation_by_offset(codebits& code, int value, int* result_data) {
	for (std::vector<relocation>::iterator i = code.relocations.begin(); i != code.relocations.end(); ++i) {
		if (i->offset == value) {
			int32_t* dataptr = (int32_t*)(&code.bytes[i->offset]);
			*result_data = *dataptr;
			return &*i;
		}
	}
	return 0;
}

int bof_find_max_relocation_offset(codebits& code) {
	// TODO: SHOUL REALLY PAD SECTIONS WHEN MERGING!!!!! PAD by 2 or something!!!
	// must avoid getting any relocations at 1byte before end of section!
	int maxofs = 0;
	for (std::vector<relocation>::iterator i = code.relocations.begin(); i != code.relocations.end(); ++i) {
		if (i->offset > maxofs) maxofs = i->offset;
	}
	return maxofs;
}

const pe_import* bof_find_import(const std::vector<pe_import>& imports, const std::string& trigger_symbol) {
	for (std::vector<pe_import>::const_iterator i = imports.begin(); i != imports.end(); ++i) {
		if (i->trigger_symbol == trigger_symbol || i->import_symbol == trigger_symbol) {
			return &*i;
		}
	}
	return 0;
}

void bof_get_undefined_symbols(binary_object_file& bof, std::vector<std::string>& result) {
	for (std::vector<symbol>::iterator i = bof.symbols.begin(); i != bof.symbols.end(); ++i) {
		if (i->type == symbol_none) {
			result.push_back(i->name);
		}
	}
}

void bof_rename_symbol(binary_object_file& bof, const std::string& oldname, const std::string& newname, int section_index) {
	// can be duplicate symbols in the same bof, but in different section, rename by section
	for (std::vector<symbol>::iterator j = bof.symbols.begin(); j != bof.symbols.end(); ++j) {
		if (j->name == oldname && j->section_index == section_index) {
			j->name = newname;
			codebits* code = bof_find_code(bof, j->section, j->section_index);
			assert(code != 0);
			for (std::vector<relocation>::iterator j = code->relocations.begin(); j != code->relocations.end(); ++j) {
				if (j->symbol == oldname) {
					j->symbol = newname;
				}
			}
		}
	}
}

std::string bof_get_new_symbol_name(binary_object_file& bof, const std::string& prefix) {
	int counter = 1;
	while (true) {
		std::stringstream strm;
		strm << prefix << "_" << counter;
		symbol* s = bof_find_symbol(bof, strm.str());
		if (s == 0) {
			return strm.str();
		}
		counter ++;
	}
}

bool bof_add(binary_object_file& dest, const binary_object_file& src) {
	// there can be multiple .text sections in src coffs, these are merged
	// into the same .text section in dest

	// make sure all code sections exist in dest

	for (std::vector<codebits>::const_iterator i = src.code.begin(); i != src.code.end(); ++i) {
		codebits* destcode = bof_find_code(dest, i->name, -1);
		if (destcode == 0) {
			// add blank section, bytes will be merged below
			codebits srccode;
			srccode.is_executable = i->is_executable;
			srccode.is_readable = i->is_readable;
			srccode.is_writable = i->is_writable;
			srccode.is_uninitialized = i->is_uninitialized;
			srccode.index = 0;
			srccode.name = i->name;
			dest.code.push_back(srccode);
		}
	}

	// allowed, duplicate symbols to ignore. need to track these to avoid 
	// adjusting offsets for an existing symbol when merging bytes
	// there can be duplicate symbols in the same bof in different sections
	std::vector<std::pair<std::pair<std::string, int>, std::string> > rename_symbols;

	// copy/resolve symbols from src in dest
	for (std::vector<symbol>::const_iterator j = src.symbols.begin(); j != src.symbols.end(); ++j) {

		symbol* destsym = bof_find_symbol(dest, j->name);

		if (j->type == symbol_func || j->type == symbol_object) {
			if (destsym == 0) {
				// exists in src, but not dest
				symbol newsym = *j;
				newsym.section_index = 0;
				dest.symbols.push_back(newsym);
			} else if (destsym->type == symbol_none) {
				// exists in src, exists but unresolved in dest
				destsym->is_global = j->is_global;
				destsym->import_module = j->import_module;
				destsym->section = j->section;
				destsym->section_index = 0;
				destsym->value = j->value;
				destsym->size = j->size;
				destsym->type = j->type;
			} else {
				// duplicate symbol definition: real constants, static
				// anonymous data ++ are permitted duplicate.
				// f.ex switch tables can have same id, but different content
				// so we need renaming or something more clever. rename for now

				symbol newsym = *j;
				newsym.name = bof_get_new_symbol_name(dest, j->name);
				newsym.section_index = 0;
				dest.symbols.push_back(newsym);

				rename_symbols.push_back(std::pair<std::pair<std::string, int>, std::string>(std::pair<std::string, int>(j->name, j->section_index), newsym.name));
				if (j->is_global) 
					std::cout << "global symbol ";
				else
					std::cout << "local symbol ";
				std::cout << j->name << " is duplicate. renamed to " << newsym.name << std::endl;
				//rename_relocations(src, j->name, newsym.name);
			}
		} else if (j->type == symbol_none) {
			if (destsym == 0) {
				symbol newsym = *j;
				newsym.section_index = 0;
				dest.symbols.push_back(newsym);
			}
		} else if (j->type == symbol_section) {
			if (destsym == 0) {
				symbol newsym = *j;
				newsym.section_index = 0;
				dest.symbols.push_back(newsym);
			} else {
				assert(destsym->type == symbol_section);
				assert(destsym->is_global == j->is_global);
			}
		} else {
			assert(false);
		}
	}

	// create a copy of the const src where symbols can be renamed before merging
	binary_object_file srccopy = src;
	for (std::vector<std::pair<std::pair<std::string, int>, std::string> >::iterator i = rename_symbols.begin(); i != rename_symbols.end(); ++i) {
		bof_rename_symbol(srccopy, i->first.first, i->second, i->first.second);
	}

	// combine sections with the same name, add new sections
	for (std::vector<codebits>::iterator i = srccopy.code.begin(); i != srccopy.code.end(); ++i) {
		codebits* destcode = bof_find_code(dest, i->name, -1);
		assert(destcode != 0);

		// pad with nulls such that the merged section is aligned,
		// meant to avoid creating relocations overlapping a section alignment
		
		// pad to 4k IF: src is > 4k, or (dest+src) overlaps a 4k segment
		// 4k = pe alignment

		int alignment = 4096;
		int padsize = 8;
		int alignnewsize = (destcode->bytes.size() + i->bytes.size()) % alignment;
		int alignoldsize = (destcode->bytes.size()) % alignment;

		if (i->bytes.size() >= alignment || (alignoldsize > alignnewsize)) {
			padsize = alignment;
		}
		std::vector<char>::size_type sizemod = destcode->bytes.size() % padsize;
		if (sizemod > 0) {
			std::vector<char> pad(padsize - sizemod);
			destcode->bytes.insert(destcode->bytes.end(), pad.begin(), pad.end());
		}

		std::vector<char>::size_type offset = destcode->bytes.size();

		// merge code, relocations and symbols in this section
		destcode->bytes.insert(destcode->bytes.end(), i->bytes.begin(), i->bytes.end());

		for (std::vector<relocation>::iterator j = i->relocations.begin(); j != i->relocations.end(); ++j) {
			relocation r = *j;
			r.offset += offset;
			destcode->relocations.push_back(r);
		}

		for (std::vector<symbol>::iterator j = srccopy.symbols.begin(); j != srccopy.symbols.end(); ++j) {
			if (j->type == symbol_object || j->type == symbol_func) {
				if (j->section == i->name && j->section_index == i->index) {
					/*if (std::find(ignore_symbols.begin(), ignore_symbols.end(), j->name) != ignore_symbols.end()) {
						continue;
					}*/
					//std::cout << "adding " << j->name << std::endl;
					symbol* destsym = bof_find_symbol(dest, j->name);
					destsym->value += offset;
				}
			}
		}

	}
	return true;
}

void bof_add_exports(binary_object_file& bof, const std::vector<std::string>& exports) {
	for (std::vector<std::string>::const_iterator i = exports.begin(); i != exports.end(); ++i) {
		symbol* sym = bof_find_symbol(bof, *i);
		assert(sym != 0);
		sym->is_export = true;
	}
}

bool bof_try_load_object_file(const std::string& filename, const std::vector<std::string>& search_paths, std::string& result_path, binary_object_file& result) {
	// currently does not care about loading from absolute paths, so just try all search paths and hope for the best
	for (std::vector<std::string>::const_iterator i = search_paths.begin(); i != search_paths.end(); ++i) {
		std::string path = *i;
		if (*path.rbegin() != '/' && *path.rbegin() != '\\') {
			path += "/";
		}
		path += filename;
		
		// try elf first, because coffs have less magic to probe
		if (bof_load_elf(path, result)) {
			result_path = path;
			return true;
		}

		if (bof_load_coff(path, result)) {
			result_path = path;
			return true;
		}
	}
	return false;
}

// simple helper to save a few lines
bool bof_get_code_and_symbol(binary_object_file& bof, const std::string& name, codebits** resultcode, symbol** resultsym) {
	symbol* sym = bof_find_symbol(bof, name);
	if (sym == 0) {
		return false;
	}

	codebits* symcode = bof_find_code(bof, sym->section, sym->section_index);
	if (symcode == 0) {
		return false;
	}

	*resultcode = symcode;
	*resultsym = sym;
	return true;
}
