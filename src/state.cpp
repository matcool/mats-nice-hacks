#include "state.hpp"
#include <filesystem>
#include <fstream>
#include "utils.hpp"

StateStruct& state() {
	static StateStruct inst;
	return inst;
}

auto get_save_path() {
	return get_exe_path() / "mathacks.txt";
}

void StateStruct::load() {
	const auto path = get_save_path();
	if (std::filesystem::exists(path)) {
		std::ifstream file(path);
		std::unordered_map<std::string, std::string> values;
		std::string line;
		while (std::getline(file, line)) {
			if (!line.empty())
				values.insert(split_once(line, '='));
		}

		load_schema_loop<0>(this, values);
	}
}

void StateStruct::save() {
	std::ofstream file(get_save_path());

	save_schema_loop<0>(this, file);
}
