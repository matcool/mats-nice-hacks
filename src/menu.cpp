#include "menu.hpp"
#include <imgui-hook.hpp>
#include "state.hpp"
#include <MinHook.h>
#include "utils.hpp"
#include "gd.hpp"
#include "explorer.hpp"

void update_speed_hack() {
	const auto value = state().speed_hack_enabled ? state().speed : 1.f;
	if (auto fme = FMODAudioEngine::sharedState())
		if (auto sound = fme->currentSound())
			sound->setPitch(value);
	CCDirector::sharedDirector()->m_pScheduler->setTimeScale(value);
}

void update_fps_bypass() {
	const auto value = state().fps_bypass ? state().fps : 60.f;
	static const auto addr = cocos_symbol<&CCApplication::setAnimationInterval>("?setAnimationInterval@CCApplication@cocos2d@@UAEXN@Z");
	addr(CCApplication::sharedApplication(), 1.0 / value);
	CCDirector::sharedDirector()->setAnimationInterval(1.0 / value);
}

void patch_toggle(uintptr_t addr, const std::vector<uint8_t>& bytes, bool replace) {
	static std::unordered_map<uintptr_t, uint8_t*> values;
	if (values.find(addr) == values.end()) {
		auto data = new uint8_t[bytes.size()];
		memcpy(data, reinterpret_cast<void*>(addr), bytes.size());
		values[addr] = data;
	}
	DWORD old_prot;
	VirtualProtect(reinterpret_cast<void*>(addr), bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot);
	memcpy(reinterpret_cast<void*>(addr), replace ? bytes.data() : values[addr], bytes.size());
	VirtualProtect(reinterpret_cast<void*>(addr), bytes.size(), old_prot, &old_prot);
}

void imgui_render() {
	const bool force = state().just_loaded;
	const auto& font = ImGui::GetIO().Fonts->Fonts.back();
	ImGui::PushFont(font);
	if (state().visible || force) {

		// ImGui::ShowDemoWindow();
		auto frame_size = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize();

		constexpr float border = 25;
		ImGui::SetNextWindowPos({ border, border });
		ImGui::SetNextWindowSizeConstraints({0, 0}, {frame_size.width, frame_size.height - border * 2.f});

		if (ImGui::Begin("mat's nice hacks", nullptr,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::MenuItem("Explorer"))
					state().explorer_enabled = !state().explorer_enabled;
				ImGui::EndMenuBar();
			}

			ImGui::SetNextItemWidth(120);
			if (ImGui::DragFloat("##speedhack", &state().speed, 0.05f, 0.001f, 10.f))
				update_speed_hack();
			ImGui::SameLine();
			if (ImGui::Checkbox("Speedhack", &state().speed_hack_enabled))
				update_speed_hack();

			ImGui::SetNextItemWidth(120);
			if (ImGui::InputFloat("##fpsbypass", &state().fps))
				update_fps_bypass();
			ImGui::SameLine();
			if (ImGui::Checkbox("FPS bypass", &state().fps_bypass))
				update_fps_bypass();

			ImGui::Checkbox("Retry keybind (R)", &state().has_retry_keybind);
			if (ImGui::Checkbox("No transition", &state().no_transition) || force) {
				// patch
				//   movss dword ptr [esi + 0xf0], xmm0
				// to
				//   xor eax, eax
				//   mov dword ptr [esi + 0xf0], eax
				patch_toggle(cocos_base + 0xa49a7, { 0x31, 0xc0, 0x89, 0x86, 0xf0, 0x00, 0x00, 0x00 }, state().no_transition);
			}
			if (ImGui::Checkbox("No trail", &state().no_trail) || force) {
				// CCMotionStreak::draw
				patch_toggle(cocos_base + 0xac080, { 0xc3 }, state().no_trail);
			}
			if (ImGui::Checkbox("No wave trail", &state().no_wave_trail) || force) {
				// PlayerObject::activateStreak
				patch_toggle(base + 0xe0d54, { 0xeb }, state().no_wave_trail);
			}
			if (ImGui::Checkbox("Solid wave trail", &state().solid_wave_trail) || force) {
				// PlayerObject::setupStreak
				patch_toggle(base + 0xd9ade, { 0x90, 0x90 }, state().solid_wave_trail);
			}
			if (ImGui::Checkbox("No particles", &state().no_particles) || force) {
				// CCParticleSystemQuad::draw
				patch_toggle(cocos_base + 0xb77f0, { 0xc3 }, state().no_particles);
			}
			if (ImGui::Checkbox("Copy hack", &state().copy_hack) || force) {
				// LevelInfoLayer::init and LevelInfoLayer::tryClone
				patch_toggle(base + 0x9c7ed, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }, state().copy_hack);
				patch_toggle(base + 0x9dfe5, { 0x90, 0x90 }, state().copy_hack);
			}
			if (ImGui::Checkbox("Hide practice buttons", &state().hide_practice) || force) {
				// UILayer::init, replaces two calls with add esp, 4
				patch_toggle(base + 0xfee29, { 0x83, 0xc4, 0x04, 0x90, 0x90, 0x90 }, state().hide_practice);
				patch_toggle(base + 0xfee6a, { 0x83, 0xc4, 0x04, 0x90, 0x90, 0x90 }, state().hide_practice);
			}
			ImGui::Checkbox("Show percent", &state().show_percent);
			if (ImGui::Checkbox("Verify hack", &state().verify_hack) || force) {
				// EditLevelLayer::onShare
				patch_toggle(base + 0x3d760, { 0xeb }, state().verify_hack);
			}
			ImGui::Checkbox("Hide attempts", &state().hide_attempts);
			ImGui::Checkbox("Hide player", &state().hide_player);
			ImGui::Checkbox("Editor preview mode", &state().preview_mode);
			if (ImGui::Checkbox("Edit level", &state().edit_level) || force) {
				// PauseLayer::init
				patch_toggle(base + 0xd62ef, { 0x90, 0x90 }, state().edit_level);
			}
			if (ImGui::Checkbox("Hide trigger lines", &state().hide_trigger_lines) || force) {
				// DrawGridLayer::draw
				patch_toggle(base + 0x93e08, { 0xE9, 0xCE, 0x00, 0x00, 0x00, 0x90 }, state().hide_trigger_lines);
			}
			if (ImGui::Checkbox("Hide grid", &state().hide_grid) || force) {
				// DrawGridLayer::draw
				// gets rid of the line at the start and the ground line but oh well 
				// setting the opacity to 0 didnt work if u zoomed in it was weird
				patch_toggle(base + 0x938a0, { 0xe9, 0x5a, 0x01, 0x00, 0x00, 0x90 }, state().hide_grid);
				patch_toggle(base + 0x93a4a, { 0xe9, 0x54, 0x01, 0x00, 0x00, 0x90 }, state().hide_grid);
			}
			if (ImGui::Checkbox("Smooth editor trail", &state().smooth_editor_trail) || force) {
				// LevelEditorLayer::update
				patch_toggle(base + 0x91a34, { 0x90, 0x90 }, state().smooth_editor_trail);
			}
			ImGui::Checkbox("Always fix yellow color bug", &state().always_fix_hue);
		}
		ImGui::End();

		if (state().explorer_enabled)
			render_explorer_window(state().explorer_enabled);

	}
	state().just_loaded = false;
	// this is so hacky lmao
	// so for some reason when you alt tab, the fps bypass resets to 60fps
	// i woudlve debugged it to see where it came from, but for some reason
	// 1.9 has antidebugging and i dont feel like figuring out how to bypass it
	if (state().fps_bypass && CCDirector::sharedDirector()->getAnimationInterval() != (1.0 / state().fps)) {
		update_fps_bypass();
	}

	if (state().color_picker) {
		static bool just_opened = true;
		static Color3F color;
		if (just_opened) {
			ImGui::OpenPopup("Color picker");
			just_opened = false;
			color = Color3F::from(state().color_picker->getColorValue());
		}
		bool unused_open = true;
		if (ImGui::BeginPopupModal("Color picker", &unused_open,
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {

			ImGui::ColorPicker3("##color_picker.color", &color.r);

			ImGui::EndPopup();
		} else {
			state().color_picker->setColorValue(color);
			state().color_picker = nullptr;
			just_opened = true;
		}
	}

	if (ImGui::GetTime() < 5.0 && !state().visible) {
		ImGui::SetNextWindowPos({ 10, 10 });
		ImGui::Begin("startupmsgwindow", nullptr,
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("Mat's nice hacks loaded, press F1 or ~ to show menu");
		ImGui::End();
	}

	ImGui::PopFont();
}

void imgui_init() {
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Muli-SemiBold.ttf", 18.f);

	auto& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowBorderSize = 0;

	auto colors = style.Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(0.31f, 0.31f, 0.31f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.59f, 0.59f, 0.59f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.61f, 0.61f, 0.61f, 0.67f);
	colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.89f, 0.89f, 0.89f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.71f, 0.71f, 0.71f, 0.35f);
}

void setup_imgui_menu() {
	ImGuiHook::setToggleCallback([]() { state().visible = !state().visible; });
	ImGuiHook::setRenderFunction(imgui_render);
	ImGuiHook::setInitFunction(imgui_init);

	ImGuiHook::setupHooks([](auto addr, auto hook, auto orig) {
		MH_CreateHook(addr, hook, orig);
		MH_EnableHook(addr);
	});
}