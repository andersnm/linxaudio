#pragma once

struct context {
	std::vector<std::string> search_paths;
};

struct project_pin {
	std::string name;
	std::string vertex;
	std::string pin;
	int pin_index;
	enum linx_pin_group pin_group;
	enum linx_pin_type pin_type;
};

struct project_factory {
	std::string name;
	std::string symbol;
	std::string file;
	std::vector<std::string> deps;

	// resolved from object files:
	std::string fullpath;
	std::vector<project_pin> pins;
	std::vector<project_pin> subgraph_pins;
};

struct project_target {
	std::string name;
	std::string postfix;
	std::vector<std::string> deps;
	std::vector<std::string> exports;
	std::vector<std::string> search_paths;
};

struct project_value {
	std::string name;
	double value;
	int pin_index;
	enum linx_pin_group pin_group;
	enum linx_pin_type pin_type;
};

struct project_vertex {
	int index;
	std::string name;
	std::string factory;
	std::string subgraph;
	std::vector<project_value> init_values;
};

struct project_edge {
	std::string from_vertex;
	std::string from_pin;
	std::string to_vertex;
	std::string to_pin;

	// resolved from object files:
	int from_pin_index;
	int to_pin_index;
};

struct project_graph {
	std::string name;
	std::vector<project_vertex> vertices;
	std::vector<project_edge> edges;
	std::vector<project_pin> pins;
};

struct project_system {
	std::string author;
	std::string product;
	unsigned int uniqueId;
};

struct project {
	std::string path;
	std::vector<project_factory> factories;
	std::vector<project_target> targets;
	project_system system;
	project_graph graph;
	std::vector<project_graph> subgraphs;
};

bool project_parse_json(const std::string& jsonfile, std::string& error_message, project& result);
project_target* project_load_json(project& project, const std::string& filename, const std::string& target_name, std::string& out_errormessage);

void project_find_factory_names(project& proj, std::vector<std::string>& result);
project_factory* project_find_factory(project& proj, const std::string& name);
project_target* project_find_target(project& proj, const std::string& name);
project_graph* project_find_subgraph(project& proj, const std::string& name);
project_vertex* project_find_vertex(project_graph& graph, const std::string& name);
project_pin* project_find_pin(project_factory& factory, project_graph& graph, const std::string& name);
bool project_resolve_graph_edges(project& proj, project_graph& graph, project_vertex* parent, std::string& error_message);
//bool project_load_pins(project& proj, const std::vector<std::string>& search_paths, std::string& error_message);
bool project_load_pins(project& proj, const project_target* target, const std::string& jsonpath, std::string& error_message);
//void project_write_x86_object(project& proj, binary_object_file& result);
int project_save_pe(project& proj, project_target* target, const char* filename, std::string& out_errormessage);

