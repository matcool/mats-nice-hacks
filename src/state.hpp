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
	bool hide_trigger_lines = false;
	bool smooth_editor_trail = false;
	bool hide_grid = false;

	bool preview_mode = false;
	// fixes the piss yellow color u get sometimes
	// its a 1.9 bug thats fixed in 2.1 however
	// this only turns on on the editor color picker
	bool should_fix_hue = false;

	void load();
	void save();
};

StateStruct& state();

DEF_SCHEMA(StateStruct, speed, has_retry_keybind,
	no_transition, no_trail, no_wave_trail, solid_wave_trail,
	no_particles, copy_hack, fps_bypass, fps,
	hide_practice, show_percent, verify_hack,
	hide_attempts, edit_level, hide_trigger_lines, hide_grid,
	smooth_editor_trail)
