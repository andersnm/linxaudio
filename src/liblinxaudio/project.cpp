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
//#include "picojson.h"

void project_find_factory_names(const std::vector<project_vertex>& vertices, std::vector<std::string>& result) {
	for (std::vector<project_vertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i) {
		if (!i->factory.empty() && std::find(result.begin(), result.end(), i->factory) == result.end()) {
			result.push_back(i->factory);
		}
	}
}

void project_find_factory_names(project& proj, std::vector<std::string>& result) {
	project_find_factory_names(proj.graph.vertices, result);
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		project_find_factory_names(i->vertices, result);
	}
}

project_factory* project_find_factory(project& proj, const std::string& name) {
	for (std::vector<project_factory>::iterator i = proj.factories.begin(); i != proj.factories.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

project_target* project_find_target(project& proj, const std::string& name) {
	for (std::vector<project_target>::iterator i = proj.targets.begin(); i != proj.targets.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

project_vertex* project_find_vertex(project_graph& graph, const std::string& name) {
	for (std::vector<project_vertex>::iterator i = graph.vertices.begin(); i != graph.vertices.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

project_graph* project_find_subgraph(project& proj, const std::string& name) {
	for (std::vector<project_graph>::iterator i = proj.subgraphs.begin(); i != proj.subgraphs.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

project_pin* project_find_pin(project_factory& factory, project_graph* subgraph, const std::string& name) {
	for (std::vector<project_pin>::iterator i = factory.pins.begin(); i != factory.pins.end(); ++i) {
		if (i->name == name) return &*i;
	}
	if (subgraph) {
		for (std::vector<project_pin>::iterator i = subgraph->pins.begin(); i != subgraph->pins.end(); ++i) {
			if (i->name == name) return &*i;
		}
	}
	return 0;
}

project_pin* project_find_parent_pin(project_factory& factory, const std::string& name) {
	for (std::vector<project_pin>::iterator i = factory.subgraph_pins.begin(); i != factory.subgraph_pins.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

struct linx_pin_raw32 {
	uint32_t name;
	enum linx_pin_type pin_type;
	float min_value;
	float max_value;
	float default_value;
	float precision;
};

struct linx_pin_raw64 {
	uint64_t name;
	enum linx_pin_type pin_type;
	float min_value;
	float max_value;
	float default_value;
	float precision;
};

template <typename linx_pinT>
bool project_load_pinsT(binary_object_file& factoryfile, project_factory* pf, symbol* sym, codebits* symcode, uintptr_t paramsoffset, int pin_count, std::string& error_message, std::vector<project_pin>& result) {
	std::stringstream error;
	int relocation_data;
	relocation* paramrelo = bof_find_relocation_by_offset(*symcode, paramsoffset, &relocation_data);
	if (paramrelo == 0) {
		error << "cannot resolve pins for '" << pf->symbol << "' in object file '" << pf->file << "' for factory '" << pf->name << "'";
		error_message = error.str();
		return false;
	}
	assert(paramrelo->type == r_32 || paramrelo->type == r_64_addr64);

	symbol* paramsym;
	codebits* paramcode;
	if (!bof_get_code_and_symbol(factoryfile, paramrelo->symbol, &paramcode, &paramsym)) {
		error << "cannot resolve pins symbol for '" << pf->symbol << "' in object file '" << pf->file << "' for factory '" << pf->name << "'";
		error_message = error.str();
		return false;
	}

	linx_pinT* symparam = (linx_pinT*)&paramcode->bytes[relocation_data + paramsym->value];
	uintptr_t nameoffset = paramsym->value;
		
	for (int j = 0; j < pin_count; j++) {

		relocation* namerelo = bof_find_relocation_by_offset(*paramcode, nameoffset, &relocation_data);
		if (namerelo == 0) {
			error << "cannot resolve name";
			error_message = error.str();
			return false;
		}
		assert(namerelo->type == r_32 || namerelo->type == r_64_addr64);

		symbol* namesym;
		codebits* namecode;
		if (!bof_get_code_and_symbol(factoryfile, namerelo->symbol, &namecode, &namesym)) {
			error << "cannot resolve name symbol " << namerelo->symbol;
			error_message = error.str();
			return false;
		}

		const char* name = &namecode->bytes[relocation_data + namesym->value];
		assert(name != 0);

		project_pin pp;
		pp.pin_index = (int)result.size();
		pp.name = name;
		pp.pin_group = linx_pin_group_module;
		pp.pin_type = symparam->pin_type;
		result.push_back(pp);

		symparam ++;
		nameoffset += sizeof(linx_pinT);
	}
	return true;
}

struct linx_factory_raw32 {
	unsigned int version_size;
	uint32_t name;
	unsigned int flags;
	unsigned int pin_count;
	uint32_t pins;
	unsigned int subgraph_pin_count;
	uint32_t subgraph_pins;
	uint32_t create;
	uint32_t destroy;
	uint32_t process;
	uint32_t describe_value;
};

struct linx_factory_raw64 {
	unsigned int version_size;
	uint64_t name;
	unsigned int flags;
	unsigned int pin_count;
	uint64_t pins;
	unsigned int subgraph_pin_count;
	uint64_t subgraph_pins;
	uint64_t create;
	uint64_t destroy;
	uint64_t process;
	uint64_t describe_value;
};

template <typename linx_factoryT, typename linx_pinT>
bool project_load_pins_factoryT(binary_object_file& factoryfile, project_factory* pf, codebits* symcode, symbol* sym, std::string& error_message) {
	std::stringstream error;
	linx_factoryT* symfac = (linx_factoryT*)&symcode->bytes[sym->value];
	if (symfac->version_size != sizeof(linx_factoryT)) {
		error << "unexpected version of '" << pf->symbol << "' in object file '" << pf->file << "' for factory '" << pf->name << "'";
		error_message = error.str();
		return false;
	}

	if (symfac->pin_count > 0) {
		uintptr_t paramsoffset = sym->value + offsetof(linx_factoryT, pins);
		if (!project_load_pinsT<linx_pinT>(factoryfile, pf, sym, symcode, paramsoffset, symfac->pin_count, error_message, pf->pins)) {
			return false;
		}
	}
	if (symfac->subgraph_pin_count > 0) {
		uintptr_t subgraph_paramsoffset = sym->value + offsetof(linx_factoryT, subgraph_pins);
		if (!project_load_pinsT<linx_pinT>(factoryfile, pf, sym, symcode, subgraph_paramsoffset, symfac->subgraph_pin_count, error_message, pf->subgraph_pins)) {
			return false;
		}
	}
	return true;
}

bool project_load_pins(project& proj, const project_target* target, const std::string& jsonpath, std::string& error_message) {

	std::vector<std::string> search_paths;
	for (std::vector<std::string>::const_iterator j = target->search_paths.begin(); j != target->search_paths.end(); ++j) {
		search_paths.push_back(jsonpath + *j);
	}

	if (search_paths.empty()) {
		error_message = "the list of search paths is empty. cannot continue.";
		return false;
	}

	std::vector<std::string> factories;
	project_find_factory_names(proj, factories);
	std::stringstream error;
	for (std::vector<std::string>::iterator i = factories.begin(); i != factories.end(); ++i) {
		project_factory* pf = project_find_factory(proj, *i);
		if (pf == 0) {
			error << "cannot find factory for '" << *i << "'";
			error_message = error.str();
			return false;
		}

		binary_object_file factoryfile;
		std::string objectfilename;
		if (!bof_try_load_object_file(pf->file, search_paths, objectfilename, factoryfile)) {
			error << "cannot load object file '" << pf->file << "' for factory '" << *i << "'";
			error_message = error.str();
			return false;
		}

		pf->fullpath = objectfilename;

		symbol* sym;
		codebits* symcode;
		if (!bof_get_code_and_symbol(factoryfile, pf->symbol, &symcode, &sym)) {
			error << "cannot find symbol '" << pf->symbol << "' in object file '" << pf->file << "' for factory '" << *i << "'";
			error_message = error.str();
			return false;
		}

		if (factoryfile.arch == arch_i386) {
			if (!project_load_pins_factoryT<struct linx_factory_raw32, struct linx_pin_raw32>(factoryfile, pf, symcode, sym, error_message)) {
				return false;
			}
		} else if (factoryfile.arch == arch_x64) {
			if (!project_load_pins_factoryT<struct linx_factory_raw64, struct linx_pin_raw64>(factoryfile, pf, symcode, sym, error_message)) {
				return false;
			}
		} else {
			assert(false);
		}

	}

	return true;
}

project_pin* project_resolve_vertex_pin(project& proj, project_graph& graph, project_vertex* parent, const std::string& vertex, const std::string& pin) {
	project_vertex* from_vertex;
	bool is_parent;
	if (vertex == "PARENT") {
		from_vertex = parent;
		is_parent = true;
	} else {
		from_vertex = project_find_vertex(graph, vertex);
		is_parent = false;
	}
	
	if (from_vertex == 0) {
		return 0;
	}
		
	project_factory* from_factory = project_find_factory(proj, from_vertex->factory);
	if (from_factory == 0) {
		return 0;
	}

	project_graph* subgraph = 0;
	if (!from_vertex->subgraph.empty()) {
		subgraph = project_find_subgraph(proj, from_vertex->subgraph);
	}

	if (is_parent) {
		return project_find_parent_pin(*from_factory, pin);
	} else {
		return project_find_pin(*from_factory, subgraph, pin);
	}
}

bool project_resolve_graph_edges(project& proj, project_graph& graph, project_vertex* parent, std::string& error_message) {
	std::stringstream error;
	for (std::vector<project_vertex>::iterator j = graph.vertices.begin(); j != graph.vertices.end(); ++j) {
		for (std::vector<project_value>::iterator i = j->init_values.begin(); i != j->init_values.end(); ++i) {
			project_pin* param = project_resolve_vertex_pin(proj, graph, parent, j->name, i->name);
			if (param == 0) {
				error << "unable to resolve vertex init value for pin '" << i->name << "' from '" << j->name << "'";
				error_message = error.str();
				return false;
			}
			if (param->pin_type != linx_pin_type_in_scalar_int && param->pin_type != linx_pin_type_in_scalar_float) {
				error << "vertex init value for pin '" << i->name << "' from '" << j->name << "' is not a scalar";
				error_message = error.str();
				return false;
			}

			i->pin_type = param->pin_type;
			i->pin_index = param->pin_index;
			i->pin_group = param->pin_group;
		}
	}


	for (std::vector<project_pin>::iterator i = graph.pins.begin(); i != graph.pins.end(); ++i) {
		project_pin* param = project_resolve_vertex_pin(proj, graph, parent, i->vertex, i->pin);
		if (param == 0) {
			error << "unable to resolve extra pin '" << i->pin << "' from '" << i->vertex << "'";
			error_message = error.str();
			return false;
		}
		i->pin_index = param->pin_index;
		i->pin_group = param->pin_group;
	}

	for (std::vector<project_edge>::iterator i = graph.edges.begin(); i != graph.edges.end(); ++i) {
		// lookup from_vertex and to_vertex, resolve possibly to parent
		// then find the vertex factory
		// then find the pins and write the indexes to the project edge
		project_pin* from_param = project_resolve_vertex_pin(proj, graph, parent, i->from_vertex, i->from_pin);
		project_pin* to_param = project_resolve_vertex_pin(proj, graph, parent, i->to_vertex, i->to_pin);
		if (from_param == 0) {
			// try subgraph extra pins
			error << "unable to resolve edge from_vertex/from_pin '" << i->from_pin << "' from '" << i->from_vertex << "'";
			error_message = error.str();
			return false;
		}

		if (to_param == 0) {
			// try subgraph extra pins
			error << "unable to resolve edge to_vertex/to_pin '" << i->to_pin << "' to '" << i->to_vertex << "'";
			error_message = error.str();
			return false;
		}

		if (!linx_pin_is_in(to_param->pin_type) ) {
			error << "edge input '" << i->to_pin << "' to '" << i->to_vertex << "' is not an input pin type";
			error_message = error.str();
			return false;
		}

		if (!linx_pin_is_out(from_param->pin_type) ) {
			error << "edge output '" << i->from_pin << "' to '" << i->from_vertex << "' is not an output pin type";
			error_message = error.str();
			return false;
		}

		i->from_pin_index = from_param->pin_index;
		i->to_pin_index = to_param->pin_index;
	}

	// traverse vertices and recurse into subgraph
	for (std::vector<project_vertex>::iterator i = graph.vertices.begin(); i != graph.vertices.end(); ++i) {
		if (i->subgraph.empty()) {
			continue;
		}

		project_graph* subgraph = project_find_subgraph(proj, i->subgraph);
		if (subgraph == 0) {
			error << "unable to find subgraph '" <<  i->subgraph << "' referenced by vertex '" << i->name << "'";
			error_message = error.str();
			return false;
		}
		if (!project_resolve_graph_edges(proj, *subgraph, &*i, error_message)) {
			return false;
		}

	}

	return true;
}
