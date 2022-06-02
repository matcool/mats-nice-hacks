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

template <size_t N, class T>
void load_schema_loop(T& obj, const std::unordered_map<std::string, std::string>& values) {
	using S = get_schema<T>;
	auto f = values.find(S::names[N]);
	if (f != values.end()) {
		const auto str = f->second; 
		using type = typename S::template type_at<N>;
		auto& value = S::template value_at<N>(obj);
		if constexpr (std::is_same_v<type, bool>) {
			value = str == "true";
		} else if constexpr (std::is_same_v<type, float>) {
			value = std::stof(str);
		} else {
			static_assert(!std::is_same_v<type, type>, "unsupported type");
		}
	}
	if constexpr (N < S::size() - 1)
		load_schema_loop<N + 1>(obj, values);
}

template <size_t N, class T>
void save_schema_loop(const T& obj, std::ofstream& file) {
	using S = get_schema<T>;
	using type = typename S::template type_at<N>;
	const auto& value = S::template value_at<N>(obj);
	file << S::names[N] << '=';
	if constexpr (std::is_same_v<type, bool>) {
		file << (value ? "true" : "false");
	} else if constexpr (std::is_same_v<type, float>) {
		file << value;
	} else {
		static_assert(!std::is_same_v<type, type>, "unsupported type");
	}
	file << std::endl;
	if constexpr (N < S::size() - 1)
		save_schema_loop<N + 1>(obj, file);
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

		load_schema_loop<0>(*this, values);
	}
}

void StateStruct::save() {
	std::ofstream file(get_save_path());

	save_schema_loop<0>(*this, file);
}
