#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cstddef>
#include <stdint.h>
#include "../liblinx/bof.h"
#include "../graph/linxaudio.h"
#include "project.h"
#include "picojson.h"

double get_object_double(const picojson::value& obj, const std::string& name, double defvalue) {
	picojson::value val = obj.get(name);
	if (val.is<double>()) return val.get<double>();
	return defvalue;
}

int get_object_int(const picojson::value& obj, const std::string& name, int defvalue) {
	picojson::value val = obj.get(name);
	if (val.is<int>()) return (int)val.get<double>();
	return defvalue;
}

std::string get_object_string(const picojson::value& obj, const std::string& name) {
	picojson::value val = obj.get(name);
	if (val.is<std::string>()) return val.get<std::string>();
	return "";
}

bool get_object_bool(const picojson::value& obj, const std::string& name, bool defvalue) {
	picojson::value val = obj.get(name);
	if (val.is<bool>()) return val.get<bool>();
	return defvalue;
}

bool get_object_string_array(const picojson::value& obj, const std::string& name, std::vector<std::string>& result ) {
	picojson::value val = obj.get(name);
	if (!val.is<picojson::array>()) return false;
	picojson::array arr = val.get<picojson::array>();
	for (picojson::value::array::const_iterator i = arr.begin(); i != arr.end(); ++i) {
		const picojson::value& arrval = *i;

		if (!arrval.is<std::string>()) {
			return false;
		}
		result.push_back(arrval.get<std::string>());
	}
	return true;
}

bool parse_factories(const picojson::value& factories, std::string& error_message, std::vector<project_factory>& result) {
	const picojson::value::array& factoriesarray = factories.get<picojson::array>();

	for (picojson::value::array::const_iterator i = factoriesarray.begin(); i != factoriesarray.end(); ++i) {
		const picojson::value& factory = *i;

		if (!factory.is<picojson::object>()) {
			error_message = "factories array can contain only objects";
			return false;
		}

		project_factory pf;
		pf.name = get_object_string(factory, "name");
		if (pf.name.empty()) {
			error_message = "factory name cannot be blank";
			return false;
		}
		pf.symbol = get_object_string(factory, "symbol");
		if (pf.symbol.empty()) {
			error_message = "factory symbol cannot be blank";
			return false;
		}
		pf.file = get_object_string(factory, "file");
		if (!get_object_string_array(factory, "deps", pf.deps)) {
			error_message = "factory must contain a deps array";
			return false;
		}
		result.push_back(pf);
	}
	return true;
}

bool parse_targets(const picojson::value& targets, std::string& error_message, std::vector<project_target>& result) {
	const picojson::value::array& targetsarray = targets.get<picojson::array>();

	for (picojson::value::array::const_iterator i = targetsarray.begin(); i != targetsarray.end(); ++i) {
		const picojson::value& target = *i;

		if (!target.is<picojson::object>()) {
			error_message = "targets array can contain only objects";
			return false;
		}

		project_target pt;
		pt.name = get_object_string(target, "name");
		if (pt.name.empty()) {
			error_message = "target name cannot be blank";
			return false;
		}
		pt.postfix = get_object_string(target, "postfix");
		if (pt.postfix.empty()) {
			error_message = "target postfix cannot be blank";
			return false;
		}
		if (!get_object_string_array(target, "deps", pt.deps)) {
			error_message = "target must contain a deps array";
			return false;
		}
		if (!get_object_string_array(target, "exports", pt.exports)) {
			error_message = "target must contain an exports";
			return false;
		}

		if (!get_object_string_array(target, "paths", pt.search_paths)) {
			error_message = "target must contain a paths array";
			return false;
		}

		result.push_back(pt);
	}
	return true;
}

bool parse_vertex_values(const picojson::value& vertexvalues, std::string& error_message, std::vector<project_value>& result) {
	const picojson::value::object& valuemap = vertexvalues.get<picojson::object>();

	for (picojson::object::const_iterator i = valuemap.begin(); i != valuemap.end(); ++i) {
		project_value pv;
		pv.name = i->first;
		if (i->second.is<double>()) {
			pv.value = i->second.get<double>();
		} else if (i->second.is<int>()) {
			pv.value = i->second.get<double>();
		} else {
			// TODO: check if string parses as numeric
			error_message = "vertex values array value was not numeric";
			return false;
		}
		result.push_back(pv);
	}

	return true;
}

bool parse_vertices(const picojson::value& vertices, std::string& error_message, std::vector<project_vertex>& result) {
	const picojson::value::array& verticesarray = vertices.get<picojson::array>();

	for (picojson::value::array::const_iterator i = verticesarray.begin(); i != verticesarray.end(); ++i) {
		const picojson::value& vertex = *i;

		if (!vertex.is<picojson::object>()) {
			error_message = "vertices array can only contain objects";
			return false;
		}

		project_vertex pv;
		pv.name = get_object_string(vertex, "name");
		if (pv.name.empty()) {
			error_message = "vertex name cannot be blank";
			return false;
		}
		pv.factory = get_object_string(vertex, "factory");
		pv.subgraph = get_object_string(vertex, "subgraph");
		if (pv.factory.empty()) {
			error_message = "vertex factory cannot be blank";
			return false;
		}
		pv.index = (int)result.size();

		picojson::value values = vertex.get("values");
		if (!values.is<picojson::null>()) {
			if (!values.is<picojson::object>()) {
				error_message = "vertex values must be an object of named values";
				return false;
			}
			if (!parse_vertex_values(values, error_message, pv.init_values)) {
				return false;
			}
		}
		result.push_back(pv);
	}
	return true;
}

bool parse_edges(const picojson::value& edges, std::vector<project_edge>& result) {
	const picojson::value::array& edgesarray = edges.get<picojson::array>();

	for (picojson::value::array::const_iterator i = edgesarray.begin(); i != edgesarray.end(); ++i) {
		const picojson::value& edge = *i;

		if (!edge.is<picojson::object>()) {
			return false;
		}

		project_edge pe;
		pe.from_vertex = get_object_string(edge, "from_vertex");
		pe.from_pin = get_object_string(edge, "from_pin");
		pe.to_vertex = get_object_string(edge, "to_vertex");
		pe.to_pin = get_object_string(edge, "to_pin");
		result.push_back(pe);
	}
	return true;
}

bool parse_pin_refs(const picojson::value& pins, std::vector<project_pin>& result) {
	const picojson::value::array& pinsarray = pins.get<picojson::array>();

	for (picojson::value::array::const_iterator i = pinsarray.begin(); i != pinsarray.end(); ++i) {
		const picojson::value& pin = *i;

		if (!pin.is<picojson::object>()) {
			return false;
		}

		project_pin pp;
		pp.name = get_object_string(pin, "name");
		pp.vertex = get_object_string(pin, "vertex");
		pp.pin = get_object_string(pin, "pin");
		pp.pin_group = linx_pin_group_propagated;
		pp.pin_index = result.size();
		result.push_back(pp);
	}
	return true;
}

bool parse_graph(const picojson::value& graph, std::string& error_message, project_graph& result) {

	result.name = get_object_string(graph, "name");

	picojson::value vertices = graph.get("vertices");
	if (vertices.is<picojson::null>() || !vertices.is<picojson::array>()) {
		error_message = "can not locate vertices array in graph";
		return false;
	}

	if (!parse_vertices(vertices, error_message, result.vertices)) {
		return false;
	}

	picojson::value edges = graph.get("edges");
	if (edges.is<picojson::null>() || !edges.is<picojson::array>()) {
		error_message = "can not locate edges array in graph";
		return false;
	}

	if (!parse_edges(edges, result.edges)) {
		error_message = "can not parse edges array in graph";
		return false;
	}

	picojson::value pins = graph.get("pins");
	if (pins.is<picojson::null>() || !pins.is<picojson::array>()) {
		error_message = "can not locate pins array in graph";
		return false;
	}

	if (!parse_pin_refs(pins, result.pins)) {
		error_message = "can not parse pins array in graph";
		return false;
	}

	return true;
}

bool parse_subgraphs(const picojson::value& subgraphs, std::string& error_message, std::vector<project_graph>& result) {
	const picojson::value::array& subgraphsarray = subgraphs.get<picojson::array>();

	std::vector<std::string> names;

	for (picojson::value::array::const_iterator i = subgraphsarray.begin(); i != subgraphsarray.end(); ++i) {
		const picojson::value& subgraph = *i;

		if (!subgraph.is<picojson::object>()) {
			error_message = "subgraphs array can only contain objects";
			return false;
		}

		result.push_back(project_graph());
		if (!parse_graph(subgraph, error_message, result.back())) {
			return false;
		}

		std::string& name = result.back().name;
		if (name.empty()) {
			error_message = "subgraphs must have a name";
			return false;
		}

		if (std::find(names.begin(), names.end(), name) != names.end()) {
			error_message = "subgraphs must have a unique name. found duplicate subgraph: '" + name + "'";
			return false;
		}
		names.push_back(name);
	}
	return true;
}

template<typename T>
struct charunion {
	union {
		char c[sizeof(T)];
		T v;
	};

	charunion(const char _c[]) {
		for (int i = 0; i < sizeof(T); i++)
			c[i] = _c[i];
	}

	static T convert(const char _c[]) {
		charunion<T> c(_c);
		return c.v;
	}
};

bool parse_system(const picojson::value& system, std::string& error_message, project_system& result) {

	result.author = get_object_string(system, "author");
	if (result.author.empty()) {
		error_message = "system author must be set";
		return false;
	}

	result.product = get_object_string(system, "product");
	if (result.product.empty()) {
		error_message = "system product must be set";
		return false;
	}

	// NOTE: assuming 4 char ascii string cast to uint32
	// TODO? could be interpreted as int, string, array-of-chars
	std::string uniqueId = get_object_string(system, "uniqueId");
	if (uniqueId.empty() || uniqueId.size() != 4) {
		error_message = "system uniqueId must be a string with exactly 4 ascii characters";
		return false;
	}

	result.uniqueId = charunion<int>::convert(uniqueId.c_str());
	return true;
}

bool project_parse_json(const std::string& jsonfile, std::string& error_message, project& result) {
	std::ifstream strm(jsonfile.c_str());
	if (!strm) {
		std::cerr << "cannot open " << jsonfile << std::endl;
		return false;
	}

	picojson::value root;
	error_message = picojson::parse(root, strm);

	if (!error_message.empty()) return false;	
	if (!root.is<picojson::object>()) return false;

	picojson::value system = root.get("system");
	if (system.is<picojson::null>() || !system.is<picojson::object>()) {
		error_message = "could not locate system object";
		return false;
	}

	if (!parse_system(system, error_message, result.system)) {
		return false;
	}

	picojson::value factories = root.get("factories");
	if (factories.is<picojson::null>() || !factories.is<picojson::array>()) {
		error_message = "could not locate factories array";
		return false;
	}

	if (!parse_factories(factories, error_message, result.factories)) {
		return false;
	}

	picojson::value targets = root.get("targets");
	if (targets.is<picojson::null>() || !targets.is<picojson::array>()) {
		error_message = "could not locate targets array";
		return false;
	}

	if (!parse_targets(targets, error_message, result.targets)) {
		return false;
	}

	picojson::value graph = root.get("graph");
	if (!graph.is<picojson::null>() && !graph.is<picojson::object>()) {
		error_message = "could not locate graph object";
		return false;
	}

	if (!parse_graph(graph, error_message, result.graph)) {
		return false;
	}

	if (!result.graph.name.empty()) {
		error_message = "the root graph cannot have a name";
		return false;
	}

	picojson::value subgraphs = root.get("subgraphs");
	if (!subgraphs.is<picojson::null>() && !subgraphs.is<picojson::array>()) {
		error_message = "could not locate subgraphs array";
		return false;
	}

	if (!parse_subgraphs(subgraphs, error_message, result.subgraphs)) {
		return false;
	}

	return true;
}

project_target* project_load_json(project& project, const std::string& filename, const std::string& target_name, std::string& out_errormessage) {
	std::string jsonfile = filename;
	
	project.system.uniqueId = 1234567890;
	std::string error_message;
	if (!project_parse_json(jsonfile, error_message, project)) {
		out_errormessage = error_message;
		return 0;
	}

	project_target* target = project_find_target(project, target_name.c_str());
	if (target == 0) {
		std::stringstream errorstrm;
		errorstrm << "cannot find target '" << target_name << "'" << std::endl;
		out_errormessage = errorstrm.str();
		return 0;
	}

	std::string jsonpath = "";
	std::string base_name;
	std::string::size_type ls = jsonfile.find_last_of("/\\");
	if (ls != std::string::npos) {
		jsonpath = jsonfile.substr(0, ls + 1);
		base_name = jsonfile.substr(ls + 1);
	} else {
		base_name = jsonfile;
	}

	std::string::size_type ld = base_name.find_last_of('.');
	if (ld != std::string::npos) {
		base_name = base_name.substr(0, ld);
	}

	if (!project_load_pins(project, target, jsonpath, error_message)) {
		out_errormessage = error_message;
		return 0;
	}

	if (!project_resolve_graph_edges(project, project.graph, 0, error_message)) {
		out_errormessage = error_message;
		return 0;
	}

	project.path = jsonpath;

	return target;
}
