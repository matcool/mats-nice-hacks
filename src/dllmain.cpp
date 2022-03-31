#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <cocos2d.h>
#include <matdash.hpp>
#include <matdash/minhook.hpp>
#include <matdash/boilerplate.hpp>
#include "console.hpp"
#include "gd.hpp"
#include <filesystem>
#include <algorithm>
#include "utils.hpp"
#include "state.hpp"
#include "menu.hpp"
#include "preview-mode.hpp"
#include "save-file.hpp"
#include "lvl-share.hpp"

using namespace matdash;

cc::stdcall<int> FMOD_setVolume(FMOD::Channel* sound, float v) {
	if (state().speed_hack_enabled && v >= 0.f)
		sound->setPitch(state().speed);
	return orig<&FMOD_setVolume>(sound, v);
}

void CCKeyboardDispatcher_dispatchKeyboardMSG(CCKeyboardDispatcher* self, int key, bool down) {
	if (down) {
		if (key == 'R' && state().has_retry_keybind) {
			auto pl = GameManager::sharedState()->getPlayLayer();
			if (pl) {
				pl->resetLevel();
				return;
			}
		}
	}
	orig<&CCKeyboardDispatcher_dispatchKeyboardMSG>(self, key, down);
}

CCPoint* EditorUI_getLimitedPosition(CCPoint* retVal, CCPoint point) {
	*retVal = point;
	return retVal;
}

void PlayLayer_spawnPlayer2(PlayLayer* self) {
	self->player2()->setVisible(true);
	orig<&PlayLayer_spawnPlayer2>(self);
}

static bool g_holding_in_editor = false;

void EditorUI_onPlaytest(EditorUI* self, void* btn) {
	println("onPlaytest {}", g_holding_in_editor);
	if (!g_holding_in_editor)
		return orig<&EditorUI_onPlaytest>(self, btn);
}

void EditorUI_ccTouchBegan(EditorUI* self, void* idc, void* idc2) {
	g_holding_in_editor = true;
	return orig<&EditorUI_ccTouchBegan>(self, idc, idc2);
}

void EditorUI_ccTouchEnded(EditorUI* self, void* idc, void* idc2) {
	g_holding_in_editor = false;
	return orig<&EditorUI_ccTouchEnded>(self, idc, idc2);
}

bool PlayLayer_init(PlayLayer* self, GJGameLevel* level) {
	if (!orig<&PlayLayer_init>(self, level)) return false;

	const auto win_size = CCDirector::sharedDirector()->getWinSize();

	if (state().show_percent) {
		auto gm = GameManager::sharedState();
		const auto bar = gm->getShowProgressBar();

		auto label = CCLabelBMFont::create("0.00%", "bigFont.fnt");
		label->setAnchorPoint({ bar ? 0.f : 0.5f, 0.5f });
		label->setScale(0.5f);
		label->setZOrder(999999);
		label->setPosition({ win_size.width / 2.f + (bar ? 107.2f : 0.f), win_size.height - 8.f });
		label->setTag(12345);
		self->addChild(label);
	}

	return true;
}

cc::thiscall<void> PlayLayer_update(PlayLayer* self, float dt) {
	orig<&PlayLayer_update>(self, dt);
	auto label = self->getChildByTag(12345);
	if (label) {
		const auto value = std::min(self->player1()->getPositionX() / self->levelLength(), 1.f) * 100.f;
		reinterpret_cast<CCLabelBMFont*>(label)->setString(CCString::createWithFormat("%.2f%%", value)->getCString());
	}
	// sick
	if (state().hide_player) {
		self->player1()->setVisible(false);
		self->player2()->setVisible(false);
	}
	// this seems to reset on resetLevel, but im too lazy to go hook that
	if (state().hide_attempts)
		self->attemptsLabel()->setVisible(false);
	
	return {};
}

bool ColorSelectPopup_init(ColorSelectPopup* self, GameObject* obj, int color_id, int idk, int idk2) {
	if (!orig<&ColorSelectPopup_init>(self, obj, color_id, idk, idk2)) return false;

	constexpr auto handler = [](CCObject* _self, CCObject* button) {
		auto self = reinterpret_cast<ColorSelectPopup*>(_self);
		auto picker = self->colorPicker();
		state().color_picker = picker;
	};
	auto sprite = ButtonSprite::create("picker", 0x28, 0, 0.6f, true, "goldFont.fnt", "GJ_button_04.png", 30.0);

	auto button = CCMenuItemSpriteExtra::create(sprite, nullptr, self, to_handler<SEL_MenuHandler, handler>);
	const auto win_size = CCDirector::sharedDirector()->getWinSize();
	const auto menu = self->menu();
	button->setPosition(menu->convertToNodeSpace({ win_size.width - 50.f, win_size.height - 110 }));
	menu->addChild(button);

	return true;
}

void EditorPauseLayer_customSetup(EditorPauseLayer* self) {
	orig<&EditorPauseLayer_customSetup>(self);

	constexpr auto handler = [](CCObject* self, CCObject*) {
		if (!OpenClipboard(NULL)) return;
		if (!EmptyClipboard()) return;
		auto handle = GetClipboardData(CF_TEXT);
		if (handle) {
			auto text = static_cast<char*>(GlobalLock(handle));
			std::string str(text);
			GlobalUnlock(handle);
			auto editor = reinterpret_cast<LevelEditorLayer*>(reinterpret_cast<CCNode*>(self)->getParent());
			editor->editorUI()->pasteObjects(str);
		}
		CloseClipboard();
	};

	auto sprite = ButtonSprite::create("paste string", 0x28, 0, 0.6f, true, "goldFont.fnt", "GJ_button_04.png", 30.0);
	auto button = CCMenuItemSpriteExtra::create(sprite, nullptr, self, to_handler<SEL_MenuHandler, handler>);
	const auto win_size = CCDirector::sharedDirector()->getWinSize();
	auto menu = CCMenu::create();
	menu->setPosition({0, 0});
	button->setPosition({ win_size.width - 50.f, 300.f });
	menu->addChild(button);
	self->addChild(menu);
}

void AppDelegate_trySaveGame(AppDelegate* self) {
	orig<&AppDelegate_trySaveGame>(self);

	state().save();
}

bool EditorUI_init(EditorUI* self, LevelEditorLayer* editor) {
	if (!orig<&EditorUI_init>(self, editor)) return false;

	if (!state().saved_clipboard.empty()) {
		self->clipboard() = state().saved_clipboard;
		self->updateButtons();
	}

	return true;
}

void EditorUI_dtor(EditorUI* self) {
	state().saved_clipboard = self->clipboard();
	orig<&EditorUI_dtor>(self);
}

void LevelInfoLayer_onClone(LevelInfoLayer* self, CCObject* foo) {
	orig<&LevelInfoLayer_onClone>(self, foo);
	if (!self->shouldDownloadLevel()) {
		auto level = static_cast<GJGameLevel*>(LocalLevelManager::sharedState()->getLocalLevels()->objectAtIndex(0));
		level->songID() = self->level()->songID();
	}
}

void EditLevelLayer_onClone(EditLevelLayer* self) {
	orig<&EditLevelLayer_onClone>(self);
	// gd checks for this->field_0x130 == 0, but i have no idea what that is nor do i care
	auto level = static_cast<GJGameLevel*>(LocalLevelManager::sharedState()->getLocalLevels()->objectAtIndex(0));
	level->songID() = self->level()->songID();
}


// static Console console;

void mod_main(HMODULE) {
	// console.setup();
	std::cout << std::boolalpha;

	state().load();

	static_assert(sizeof(CCObject) == 24, "CCObject is wrong!");
	static_assert(sizeof(CCNode) == 0xe8, "CCNode size is wrong");
	static_assert(sizeof(CCNodeRGBA) == 0xf8, "CCNodeRGBA wrong");
	static_assert(sizeof(CCSprite) == 0x1b8, "CCSprite wrong");
	static_assert(sizeof(CCDirector) == 0x100, "CCDirector size is wrong!");

	static_assert(offsetof(CCDirector, m_pRunningScene) == 0xa8, "this is wrong!");
	static_assert(offsetof(CCDirector, m_pobOpenGLView) == 0x68, "This is wrong!");

	auto cocos = GetModuleHandleA("libcocos2d.dll");
	add_hook<&FMOD_setVolume>(GetProcAddress(FMOD::base, "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));
	add_hook<&CCKeyboardDispatcher_dispatchKeyboardMSG>(GetProcAddress(cocos, "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z"));
	// add_hook<&MenuLayer_init>(base + 0xaf210);
	add_hook<&EditorUI_getLimitedPosition, matdash::Stdcall>(base + 0x4b500);

	// for invisible dual fix (thx pie)
	add_hook<&PlayLayer_spawnPlayer2>(base + 0xef0d0);

	// for freeze fix (thx cos8o)
	add_hook<&EditorUI_onPlaytest>(base + 0x489c0);
	add_hook<&EditorUI_ccTouchBegan>(base + 0x4d5e0);
	add_hook<&EditorUI_ccTouchEnded>(base + 0x4de40);

	add_hook<&AppDelegate_trySaveGame>(base + 0x293f0);

	add_hook<&PlayLayer_init>(base + 0xe35d0);
	add_hook<&PlayLayer_update>(base + 0xe9360);

	add_hook<&ColorSelectPopup_init>(base + 0x29db0);

	add_hook<&EditorPauseLayer_customSetup>(base + 0x3e3d0);

	add_hook<&EditorUI_init>(base + 0x3fdc0);
	add_hook<&EditorUI_dtor>(base + 0x3fb90);

	add_hook<&LevelInfoLayer_onClone>(base + 0x9e2c0);
	add_hook<&EditLevelLayer_onClone>(base + 0x3da30);

	preview_mode::init();
	// save_file::init();
	lvl_share::init();

	setup_imgui_menu();

	// patches for the editor extension

	patch(base + 0x4ca75, { 0xeb });
	patch(base + 0x4ca45, { 0xeb });
	patch(base + 0x4779c, { 0xeb });
	patch(base + 0x477b9, { 0xeb });

	patch(base + 0x4b445, { 0xeb, 0x44 });
}
