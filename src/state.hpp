#pragma once
#include <cocos2d.h>
#include <cocos-ext.h>
#include "schema.hpp"
#include <string>
#include <unordered_map>

struct StateStruct {
	bool visible = false;
	bool just_loaded = true;
	cocos2d::extension::CCControlColourPicker* color_picker = nullptr;
	bool explorer_enabled = false;
	std::string saved_clipboard;

	float speed = 1.f;
	bool speed_hack_enabled = false;
	bool has_retry_keybind = false;
	bool no_transition = false;
	bool no_trail = false;
	bool no_wave_trail = false;
	bool solid_wave_trail = false;
	bool no_particles = false;
	bool copy_hack = false;
	bool fps_bypass = false;
	float fps = 240.f;
	bool hide_practice = false;
	bool show_percent = false;
	bool verify_hack = false;
	bool hide_attempts = false;
	bool hide_player = false;
	bool edit_level = false;

	bool preview_mode = false;

	void load();
	void save();
};

StateStruct& state();

DEF_SCHEMA(StateStruct, speed, has_retry_keybind,
	no_transition, no_trail, no_wave_trail, solid_wave_trail,
	no_particles, copy_hack, fps_bypass, fps,
	hide_practice, show_percent, verify_hack,
	hide_attempts, edit_level)

template <size_t N, class T>
void load_schema_loop(T* obj, const std::unordered_map<std::string, std::string>& values) {
	using S = get_schema<T>;
	auto f = values.find(S::names[N]);
	if (f != values.end()) {
		const auto str = f->second;
		using type = S::type_at<N>;
		auto& value = S::value_at<N>(obj);
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
void save_schema_loop(const T* obj, std::ofstream& file) {
	using S = get_schema<T>;
	using type = S::type_at<N>;
	const auto& value = S::value_at<N>(obj);
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