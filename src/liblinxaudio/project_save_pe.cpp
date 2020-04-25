#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstddef>
#include <cassert>
#include <stdint.h>
#include "../liblinx/bof.h"
#include "../graph/linxaudio.h"
#include "project.h"

struct linx_graph_definition_raw32 {
	int vertex_count;
	uint32_t vertices;
	int edge_count;
	uint32_t edges;
	int propagated_pin_count;
	uint32_t propagated_pins;
};

struct linx_graph_definition_raw64 {
	int vertex_count;
	uint64_t vertices;
	int edge_count;
	uint64_t edges;
	int propagated_pin_count;
	uint64_t propagated_pins;
};

struct linx_vertex_definition_raw32 {
	uint32_t factory;
	uint32_t subgraph;
	int init_value_count;
	uint32_t init_values;
};

struct linx_vertex_definition_raw64 {
	uint64_t factory;
	uint64_t subgraph;
	int init_value_count;
	uint64_t init_values;
};

struct linx_pin_ref_raw32 {
	int vertex;
	enum linx_pin_group pin_group;
	int pin_index;
	uint32_t name;
};

struct linx_pin_ref_raw64 {
	int vertex;
	enum linx_pin_group pin_group;
	int pin_index;
	uint64_t name;
};



pe_import system_imports[] = {
	{ "kernel32.dll", "GetProcessHeap", "_imp__GetProcessHeap@0" },
	{ "kernel32.dll", "HeapAlloc", "_imp__HeapAlloc@12" },
	{ "kernel32.dll", "HeapFree", "_imp__HeapFree@12" },
	{ "kernel32.dll", "CloseHandle", "_imp__CloseHandle@4" },
	{ "kernel32.dll", "GetLastError", "_imp__GetLastError@0" },
	{ "kernel32.dll", "VirtualProtect", "_imp__VirtualProtect@16" },
	{ "kernel32.dll", "MapViewOfFile", "_imp__MapViewOfFile@20" },
	{ "kernel32.dll", "FlushViewOfFile", "_imp__FlushViewOfFile@8" },
	{ "kernel32.dll", "UnmapViewOfFile", "_imp__UnmapViewOfFile@4" },
	{ "kernel32.dll", "VirtualLock", "_imp__VirtualLock@8" },
	{ "kernel32.dll", "VirtualUnlock", "_imp__VirtualUnlock@8" },
	{ "kernel32.dll", "CreateFileMappingA", "_imp__CreateFileMappingA@24" },
	{ "msvcrt.dll", "free", "_imp_free" },  // x64
	{ "msvcrt.dll", "free", "_imp__free" }, // x86
	{ "msvcrt.dll", "malloc", "_imp_malloc" }, // x64
	{ "msvcrt.dll", "malloc", "_imp__malloc" }, // x86
	{ "msvcrt.dll", "_snprintf", "_imp__snprintf" }, // x64
	{ "msvcrt.dll", "_snprintf", "_imp___snprintf" }, // x86
	{ "msvcrt.dll", "_errno", "_imp___errno" },
	{ "msvcrt.dll", "_get_osfhandle", "_imp___get_osfhandle" },
};

int system_import_count = sizeof(system_imports) / sizeof(pe_import);

const int graphs_section = 0;
const int vertices_section = 1;
const int edges_section = 2;
const int pins_section = 3;
const int system_section = 4;
const int values_section = 5;
const int entry_section = 6;

template <typename T>
void add_unique(std::vector<T>& vec, const T& val) {
	if (std::find(vec.begin(), vec.end(), val) == vec.end())
		vec.push_back(val);
}

template <typename T>
void add_unique(std::vector<T>& vec, const std::vector<T>& val) {
	for (typename std::vector<T>::const_iterator i = val.begin(); i != val.end(); ++i)
		add_unique(vec, *i);
}

template <typename linx_graph_definitionT, enum relocationtype reltype>
void project_write_obj_graph(project& proj, project_graph& graph, codebits& graphdata, int graphs_offset, binary_object_file& result) {

	linx_graph_definitionT g;
	memset(&g, 0, sizeof(linx_graph_definitionT));
	g.edge_count = graph.edges.size();
	g.vertex_count = graph.vertices.size();
	g.propagated_pin_count = graph.pins.size();
	memcpy(&graphdata.bytes[graphs_offset], &g, sizeof(linx_graph_definitionT));

	std::string prefix = "linx_graph_array" + graph.name;
	bof_graph_add_symbol(result, false, prefix, ".text", graphs_section, symbol_object, graphs_offset, 0);
	if (!graph.vertices.empty()) {
		bof_graph_add_relocation(graphdata, reltype, prefix + ".vertices", graphs_offset + offsetof(linx_graph_definitionT, vertices));
	}
	if (!graph.edges.empty()) {
		bof_graph_add_relocation(graphdata, reltype, prefix + ".edges", graphs_offset + offsetof(linx_graph_definitionT, edges));
	}
	if (!graph.pins.empty()) {
		bof_graph_add_relocation(graphdata, reltype, prefix + ".pins", graphs_offset + offsetof(linx_graph_definitionT, propagated_pins));
	}
}

template <typename linx_graph_definitionT, enum relocationtype reltype>
void project_write_obj_graphs(project& proj, binary_object_file& result) {
	int size_of_graphs = sizeof(linx_graph_definitionT) * (1 + proj.subgraphs.size());
	
	codebits graphdata;
	graphdata.is_executable = true;
	graphdata.is_readable = true;
	graphdata.name = ".text";
	graphdata.index = graphs_section;
	graphdata.bytes.resize(size_of_graphs);

	int graphs_offset = 0;
	project_write_obj_graph<linx_graph_definitionT, reltype>(proj, proj.graph, graphdata, graphs_offset, result);
	graphs_offset += sizeof(linx_graph_definitionT);
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		project_write_obj_graph<linx_graph_definitionT, reltype>(proj, *i, graphdata, graphs_offset, result);
		graphs_offset += sizeof(linx_graph_definitionT);
	}
	result.code.push_back(graphdata);
}

template <typename linx_vertex_definitionT, enum relocationtype reltype>
void project_write_obj_vertices(project& proj, project_graph& graph, codebits& vertexdata, int vertices_offset, binary_object_file& result) {

	// add a symbol pointing in the vertex buffer where this graphs vertices start
	std::string prefix = "linx_graph_array" + graph.name;
	bof_graph_add_symbol(result, false, prefix + ".vertices", ".text", vertices_section, symbol_object, vertices_offset, 0);

	for (std::vector<project_vertex>::iterator i = graph.vertices.begin(); i != graph.vertices.end(); ++i) {
		// each vertex contains relocated pointers and init_value_count
		linx_vertex_definitionT v;
		memset(&v, 0, sizeof(linx_vertex_definitionT));
		v.init_value_count = (int)i->init_values.size();
		
		project_factory* f = project_find_factory(proj, i->factory);
		assert(f != 0);
		bof_graph_add_relocation(vertexdata, reltype, f->symbol, vertices_offset + offsetof(linx_vertex_definitionT, factory));

		// make sure the subgraph offset is relocated into the vertex
		if (!i->subgraph.empty()) {
			std::string subprefix = "linx_graph_array" + i->subgraph;
			bof_graph_add_relocation(vertexdata, reltype, subprefix, vertices_offset + offsetof(linx_vertex_definitionT, subgraph));
		}

		if (!i->init_values.empty()) {
			std::stringstream symname;
			symname << "linx_graph_array" << graph.name << ".values" << i->index;
			bof_graph_add_relocation(vertexdata, reltype, symname.str(), vertices_offset + offsetof(linx_vertex_definitionT, init_values));
		}

		memcpy(&vertexdata.bytes[vertices_offset], &v, sizeof(linx_vertex_definitionT));
		vertices_offset += sizeof(linx_vertex_definitionT);
	}
}

template <typename linx_vertex_definitionT, enum relocationtype reltype>
void project_write_obj_vertices(project& proj, binary_object_file& result) {

	int vertex_count = proj.graph.vertices.size();
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		vertex_count += i->vertices.size();
	}

	codebits vertexdata;
	vertexdata.is_executable = true;
	vertexdata.is_readable = true;
	vertexdata.name = ".text";
	vertexdata.index = vertices_section;
	vertexdata.bytes.resize(sizeof(linx_vertex_definitionT) * vertex_count);

	int vertices_offset = 0;
	project_write_obj_vertices<linx_vertex_definitionT, reltype>(proj, proj.graph, vertexdata, 0, result);
	vertices_offset += sizeof(linx_vertex_definitionT) * proj.graph.vertices.size();

	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		project_write_obj_vertices<linx_vertex_definitionT, reltype>(proj, *i, vertexdata, vertices_offset, result);
		vertices_offset += sizeof(linx_vertex_definitionT) * i->vertices.size();
	}
	result.code.push_back(vertexdata);
}


void project_write_obj_edges(project& proj, project_graph& graph, codebits& edgedata, int edges_offset, binary_object_file& result) {

	// add a symbol pointing in the global edge buffer where this graphs edges start
	std::string prefix = "linx_graph_array" + graph.name;
	bof_graph_add_symbol(result, false, prefix + ".edges", ".text", edges_section, symbol_object, edges_offset, 0);

	for (std::vector<project_edge>::iterator i = graph.edges.begin(); i != graph.edges.end(); ++i) {
		struct linx_edge_definition e;
		if (i->from_vertex == "PARENT") {
			e.from_vertex = -1;
		} else {
			project_vertex* v = project_find_vertex(graph, i->from_vertex);
			assert(v != 0);
			e.from_vertex = v->index;
		}
		e.from_pin_index = i->from_pin_index;
		e.from_pin_group = linx_pin_group_module;

		if (i->to_vertex == "PARENT") {
			e.to_vertex = -1;
		} else {
			project_vertex* v = project_find_vertex(graph, i->to_vertex);
			assert(v != 0);
			e.to_vertex = v->index;
		}
		e.to_pin_index = i->to_pin_index;
		e.to_pin_group = linx_pin_group_module;

		memcpy(&edgedata.bytes[edges_offset], &e, sizeof(struct linx_edge_definition));
		edges_offset += sizeof(struct linx_edge_definition);
	}
}

void project_write_obj_edges(project& proj, binary_object_file& result) {

	int edge_count = proj.graph.edges.size();
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		edge_count += i->edges.size();
	}

	codebits edgedata;
	edgedata.is_executable = true;
	edgedata.is_readable = true;
	edgedata.name = ".text";
	edgedata.index = edges_section;
	edgedata.bytes.resize(sizeof(struct linx_edge_definition) * edge_count);

	int edges_offset = 0;
	project_write_obj_edges(proj, proj.graph, edgedata, 0, result);
	edges_offset += sizeof(struct linx_edge_definition) * proj.graph.edges.size();

	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		project_write_obj_edges(proj, *i, edgedata, edges_offset, result);
		edges_offset += sizeof(struct linx_edge_definition) * i->edges.size();
	}
	result.code.push_back(edgedata);
}

template <typename linx_pin_refT, enum relocationtype reltype>
void project_write_obj_pins(project& proj, project_graph& graph, codebits& pinsdata, int pins_offset, binary_object_file& result) {
	std::string prefix = "linx_graph_array" + graph.name;
	bof_graph_add_symbol(result, false, prefix + ".pins", ".text", pins_section, symbol_object, pins_offset, 0);

	for (std::vector<project_pin>::iterator i = graph.pins.begin(); i != graph.pins.end(); ++i) {

		project_vertex* v = project_find_vertex(graph, i->vertex);
		assert(v != 0);

		linx_pin_refT pref;
		memset(&pref, 0, sizeof(linx_pin_refT));
		pref.vertex = v->index;
		pref.pin_index = i->pin_index;
		pref.pin_group = i->pin_group;

		memcpy(&pinsdata.bytes[pins_offset], &pref, sizeof(linx_pin_refT));

		std::stringstream symname;
		symname << "linx_graph" << graph.name << "_proppin_name" << std::distance(graph.pins.begin(), i);
		bof_graph_add_relocation(pinsdata, reltype, symname.str(), pins_offset + offsetof(linx_pin_refT, name));

		pins_offset += sizeof(linx_pin_refT);
	}

}

template <typename linx_pin_refT, enum relocationtype reltype>
void project_write_obj_pins(project& proj, binary_object_file& result) {

	int pin_count = proj.graph.pins.size();
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		pin_count += i->pins.size();
	}

	codebits pindata;
	pindata.is_executable = true;
	pindata.is_readable = true;
	pindata.name = ".text";
	pindata.index = pins_section;
	pindata.bytes.resize(sizeof(linx_pin_refT) * pin_count);

	int pins_offset = 0;
	project_write_obj_pins<linx_pin_refT, reltype>(proj, proj.graph, pindata, 0, result);
	pins_offset += sizeof(linx_pin_refT) * proj.graph.pins.size();

	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		project_write_obj_pins<linx_pin_refT, reltype>(proj, *i, pindata, pins_offset, result);
		pins_offset += sizeof(linx_pin_refT) * i->pins.size();
	}
	result.code.push_back(pindata);
}

void project_write_obj_system(project& proj, binary_object_file& result) {
	int size_of_system = proj.system.author.length() + 1 + proj.system.product.length() + 1 + sizeof(proj.system.uniqueId);
	for (std::vector<project_pin>::iterator j = proj.graph.pins.begin(); j != proj.graph.pins.end(); ++j) {
		size_of_system += j->name.length() + 1;
	}
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		for (std::vector<project_pin>::iterator j = i->pins.begin(); j != i->pins.end(); ++j) {
			size_of_system += j->name.length() + 1;
		}
	}

	codebits systemdata;
	systemdata.is_executable = true;
	systemdata.is_readable = true;
	systemdata.name = ".text";
	systemdata.index = system_section;
	systemdata.bytes.resize(size_of_system);

	int product_offset = proj.system.author.length() + 1;
	int uniqueid_offset = product_offset + proj.system.product.length() + 1;

	memcpy(&systemdata.bytes[0], proj.system.author.c_str(), proj.system.author.length() + 1);
	bof_graph_add_symbol(result, false, "linx_graph_author", ".text", system_section, symbol_object, 0, 0);

	memcpy(&systemdata.bytes[product_offset], proj.system.product.c_str(), proj.system.product.length() + 1);
	bof_graph_add_symbol(result, false, "linx_graph_product", ".text", system_section, symbol_object, product_offset, 0);

	memcpy(&systemdata.bytes[uniqueid_offset], &proj.system.uniqueId, sizeof(unsigned int));
	bof_graph_add_symbol(result, false, "linx_graph_uniqueid", ".text", system_section, symbol_object, uniqueid_offset, 0);

	int names_offset = uniqueid_offset + sizeof(unsigned int);
	for (std::vector<project_pin>::iterator j = proj.graph.pins.begin(); j != proj.graph.pins.end(); ++j) {
		std::stringstream symname;
		symname << "linx_graph" << proj.graph.name << "_proppin_name" << std::distance(proj.graph.pins.begin(), j);

		bof_graph_add_symbol(result, false, symname.str(), ".text", system_section, symbol_object, names_offset, 0);

		strcpy(&systemdata.bytes[names_offset], j->name.c_str());
		names_offset += j->name.length() + 1;
	}
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		for (std::vector<project_pin>::iterator j = i->pins.begin(); j != i->pins.end(); ++j) {
			std::stringstream symname;
			symname << "linx_graph" << i->name << "_proppin_name" << std::distance(i->pins.begin(), j);

			bof_graph_add_symbol(result, false, symname.str(), ".text", system_section, symbol_object, names_offset, 0);

			strcpy(&systemdata.bytes[names_offset], j->name.c_str());
			names_offset += j->name.length() + 1;
		}
	}

	result.code.push_back(systemdata);
}

int project_write_obj_values(project& proj, project_graph& graph, codebits& valuesdata, int values_offset, binary_object_file& result) {

	int value_count = 0;
	for (std::vector<project_vertex>::iterator j = graph.vertices.begin(); j != graph.vertices.end(); ++j) {

		project_vertex* v = &*j;
		std::stringstream symname;
		symname << "linx_graph_array" << graph.name << ".values" << v->index;
		bof_graph_add_symbol(result, false, symname.str(), ".text", values_section, symbol_object, values_offset, 0);

		for (std::vector<project_value>::iterator i = j->init_values.begin(); i != j->init_values.end(); ++i) {
			struct linx_value value;
			value.timestamp = 0;
			value.pin_index = i->pin_index;
			value.pin_group = i->pin_group;

			if (i->pin_type == linx_pin_type_in_scalar_int) {
				value.intvalue = (int)i->value;
			} else if (i->pin_type == linx_pin_type_in_scalar_float) {
				value.floatvalue = (float)i->value;
			} else {
				assert(false);
			}

			memcpy(&valuesdata.bytes[values_offset], &value, sizeof(struct linx_value));
			values_offset += sizeof(struct linx_value);
			value_count ++;
		}
	}
	return value_count;
}

void project_write_obj_values(project& proj, binary_object_file& result) {

	int value_count = 0;
	for (std::vector<project_vertex>::iterator j = proj.graph.vertices.begin(); j != proj.graph.vertices.end(); ++j) {
		value_count += j->init_values.size();
	}
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		for (std::vector<project_vertex>::iterator j = i->vertices.begin(); j != i->vertices.end(); ++j) {
			value_count += j->init_values.size();
		}
	}

	codebits valuedata;
	valuedata.is_executable = true;
	valuedata.is_readable = true;
	valuedata.name = ".text";
	valuedata.index = values_section;
	valuedata.bytes.resize(sizeof(struct linx_value) * value_count);

	int values_offset = 0;
	value_count = project_write_obj_values(proj, proj.graph, valuedata, 0, result);
	values_offset += sizeof(struct linx_value) * value_count;

	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		value_count = project_write_obj_values(proj, *i, valuedata, values_offset, result);
		values_offset += sizeof(struct linx_value) * value_count;
	}
	result.code.push_back(valuedata);
}

template <typename linx_graph_definitionT, typename linx_vertex_definitionT, typename linx_pin_refT, enum relocationtype reltype>
void project_write_obj_object(project& proj, binary_object_file& result) {
	project_write_obj_graphs<linx_graph_definitionT, reltype>(proj, result);
	project_write_obj_vertices<linx_vertex_definitionT, reltype>(proj, result);
	project_write_obj_edges(proj, result);
	project_write_obj_pins<linx_pin_refT, reltype>(proj, result);
	project_write_obj_system(proj, result);
	project_write_obj_values(proj, result);

	// add the entrypoint, ie a pointer to the main linx_graph
	codebits entrydata;
	entrydata.is_executable = true;
	entrydata.is_readable = true;
	entrydata.name = ".text";
	entrydata.index = entry_section;
	entrydata.bytes.resize(sizeof(intptr_t));

	// point linx_graph at the start of linx_graph_array so that the main graph
	// is accessible like this:
	//         extern struct linx_graph* linx_graph;

	// it should've been possible to create a pointer-less extern to the array:
	//         extern struct linx_graph linx_graph_array;
	// but for some reason the compiled c code seems to access the struct both
	// directly and via pointer redirections. so better use the pointer:
	bof_graph_add_symbol(result, false, "linx_graph", ".text", entry_section, symbol_object, 0, 0);
	bof_graph_add_relocation(entrydata, reltype, "linx_graph_array", 0);
	result.code.push_back(entrydata);

}

bool project_prepare_pe(project& proj, project_target& target, binary_object_file& bof, std::string& error_message) {

	std::stringstream strm;

	std::vector<std::string> factory_names;
	project_find_factory_names(proj, factory_names);

	std::vector<std::string> input_files;
	for (std::vector<std::string>::iterator i = factory_names.begin(); i != factory_names.end(); ++i) {
		project_factory* f = project_find_factory(proj, *i);
		assert(f != 0);
		add_unique(input_files, f->file);
		add_unique(input_files, f->deps);
	}
	add_unique(input_files, target.deps);

	std::vector<std::string> search_paths;
	for (std::vector<std::string>::iterator i = target.search_paths.begin(); i != target.search_paths.end(); ++i) {
		search_paths.push_back(proj.path + *i);
	}

	for (int i = 0; i < input_files.size(); i++) {
		binary_object_file boflib;
		std::string result_path;
		if (!bof_try_load_object_file(input_files[i], search_paths, result_path, boflib)) {
			strm << "cant load object file " << input_files[i];
			error_message = strm.str();
			return false;
		}

		if (bof.arch == arch_none) {
			bof.arch = boflib.arch;
		} else if (boflib.arch != bof.arch) {
			strm << input_files[i] << " architecture mismatch. all object files must be of same architecture (i386, x64)";
			error_message = strm.str();
			return false;
		}

		if (!bof_add(bof, boflib)) {
			strm << "bof_add failed";
			error_message = strm.str();
			return false;
		}
	}

	if (bof.arch == arch_i386) {
		binary_object_file graph_object;
		project_write_obj_object<struct linx_graph_definition_raw32, struct linx_vertex_definition_raw32, struct linx_pin_ref_raw32, r_32> (proj, graph_object);
		bof_add(bof, graph_object);
	} else if (bof.arch == arch_x64) {
		binary_object_file graph_object;
		project_write_obj_object<struct linx_graph_definition_raw64, struct linx_vertex_definition_raw64, struct linx_pin_ref_raw64, r_64_addr64>(proj, graph_object);
		bof_add(bof, graph_object);
	} else {
		error_message = "Don't know how to compile graph for this architecture";
		return false;
	}

	bof_add_exports(bof, target.exports);

	std::vector<std::string> undefined_symbols;
	bof_get_undefined_symbols(bof, undefined_symbols);

	std::vector<pe_import> imports(system_imports, system_imports + system_import_count);

	bool has_undefined = false;
	for (std::vector<std::string>::iterator i = undefined_symbols.begin(); i != undefined_symbols.end(); ++i) {
		const pe_import* imp = bof_find_import(imports, *i);
		if (imp == 0) {
			strm << *i << " is undefined." << std::endl;
			has_undefined = true;
		}
	}

	if (has_undefined) {
		error_message = strm.str();
		return false;
	}

	return true;
}

int project_save_pe(project& proj, project_target* target, const char* filename, std::string& out_errormessage) {
	std::stringstream strm;
	binary_object_file bof;

	if (!project_prepare_pe(proj, *target, bof, out_errormessage)) {
		return 0;
	}

	std::vector<pe_import> imports(system_imports, system_imports + system_import_count);

	if (!bof_save_pe(bof, filename, imports)) {
		strm << "unable to save pe";
		out_errormessage = strm.str();
		return 0;
	}

	return 1;
}


/*
in-memory linking TODO

linxaudio_instance_t* linxaudio_project_compile(linxaudio_project_t* project, linxaudio_target_t* target, int errorlength, char* out_errormessage) {
	std::stringstream strm;
	std::string error_message;
	binary_object_file bof;
	if (!project_prepare_pe(*project, *target, bof, error_message)) {
		strncpy(out_errormessage, error_message.c_str(), errorlength);
		return 0;
	}

	std::vector<pe_import> imports(system_imports, system_imports + system_import_count);

	bof_add_imports(bof, imports);
	bof_resolve_imports(bof);

	instance* result = new instance();
	bof_save_mem(bof, imports, *result);

	return result;
}

void* linxaudio_instance_get_proc_address(linxaudio_instance_t* instance, const char* procname) {
	std::map<std::string, char*>::iterator i = instance->symbols.find(procname);
	if (i == instance->symbols.end()) {
		return 0;
	}
	return i->second;
}

void linxaudio_instance_destroy(linxaudio_instance_t* instance) {
	// TODO: munmap
	delete instance;
}
*/