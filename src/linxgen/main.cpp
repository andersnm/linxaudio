#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "../graph/linxaudio.h"
#include "../liblinxaudio/project.h"

int main(int argc, char* argv[]) {

	using std::cout;
	using std::endl;

	if (argc != 3) {
		cout << "linxgen filename.json target" << endl;
		return 0;
	}

	std::string jsonfile(argv[1]);// = "../../sample.json";

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

	std::string target_name(argv[2]);

	std::string error_message;
	project project;
	project_target* target = project_load_json(project, jsonfile, target_name, error_message);
	if (target == 0) {
		cout << error_message << endl;
		return 1;
	}

	std::string output_file = base_name + target->postfix;
	if (!project_save_pe(project, target, output_file.c_str(), error_message)) {
		cout << error_message << endl;
		return 1;
	}
	return 0;
}
