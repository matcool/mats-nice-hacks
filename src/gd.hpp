#pragma once

#define NOMINMAX
#include <cocos2d.h>
#include <cocos-ext.h>
#include <stdint.h>

using namespace cocos2d;

static const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleA(0));
static const auto cocos_base = reinterpret_cast<uintptr_t>(GetModuleHandleA("libcocos2d.dll"));

template <class R, class T>
R& from(T base, intptr_t offset) {
	return *reinterpret_cast<R*>(reinterpret_cast<uintptr_t>(base) + offset);
}

namespace gd {
	struct string {
		union {
			char inline_data[16];
			char* ptr;
		} m_data;
		size_t m_size = 0;
		size_t m_capacity = 15;

		// TODO:
		// ~string() = delete;

		string(const char* data, size_t size) {
			reinterpret_cast<void*(__thiscall*)(void*, const char*, size_t)>(base + 0xa3f0)(this, data, size);
		}

		string(const std::string& str) : string(str.c_str(), str.size()) {
		}

		size_t size() const { return m_size; }

		string& operator=(const string& other) {
			if (m_capacity > 15) delete m_data.ptr;
			reinterpret_cast<void*(__thiscall*)(void*, const char*, size_t)>(base + 0xa3f0)(this, other.c_str(), other.size());
			return *this;
		}

		const char* c_str() const {
			if (m_capacity <= 15) return m_data.inline_data;
			return m_data.ptr;
		}

		operator std::string() const {
			return std::string(c_str(), m_size);
		}
	};
}

namespace FMOD {
	static auto base = GetModuleHandleA("fmod.dll");
	struct Channel {
		void setPitch(float pitch) {
			static const auto addr = GetProcAddress(base, "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
			reinterpret_cast<void*(__stdcall*)(void*, float)>(addr)(this, pitch);
		}
	};
}

class MenuLayer : public CCLayer {
public:
};

class PlayLayer;

class GameManager : public CCNode {
public:
	static GameManager* sharedState() {
		return reinterpret_cast<GameManager*(__stdcall*)()>(base + 0x667d0)();
	}

	PlayLayer* getPlayLayer() {
		return from<PlayLayer*>(this, 0x144);
	}

	bool getShowProgressBar() {
		return from<bool>(this, 0x1d4);
	}
};

class PlayerObject;

class PlayLayer : public CCLayer {
public:
	void resetLevel() {
		return reinterpret_cast<void(__thiscall*)(PlayLayer*)>(base + 0xf1f20)(this);
	}
	// TODO: make these actual members
	auto player1() {
		return from<PlayerObject*>(this, 0x2a4);
	}
	auto player2() {
		return from<PlayerObject*>(this, 0x2a8);
	}
	auto levelLength() {
		return from<float>(this, 0x1d0);
	}
	auto attemptsLabel() {
		return from<CCLabelBMFont*>(this, 0x1d8);
	}
};

class PlayerObject : public CCSprite {
public:
	auto& position() {
		return from<CCPoint>(this, 0x4a8);
	}
};

class FMODAudioEngine : public CCNode {
public:
	static auto sharedState() {
		return reinterpret_cast<FMODAudioEngine*(__stdcall*)()>(base + 0x164c0)();
	}
	auto currentSound() {
		return from<FMOD::Channel*>(this, 0x130);
	}
};

class EditorUI : public CCLayer {
public:
	auto pasteObjects(const std::string& str) {
		return reinterpret_cast<CCArray*(__thiscall*)(EditorUI*, gd::string)>(base + 0x492a0)(this, str);
	}

	auto& clipboard() {
		return from<gd::string>(this, 0x264);
	}

	void updateButtons() {
		reinterpret_cast<void(__thiscall*)(EditorUI*)>(base + 0x41450)(this);
	}

	bool isPlayback() {
		return from<bool>(this, 0x134);
	}

	void updateZoom(float amt) {
		reinterpret_cast<void(__vectorcall*)(float, float, EditorUI*)>(base + 0x48c30)(0.f, amt, this);
	}
};

class AppDelegate : public CCApplication {

};

class GJGameLevel : public CCNode {
public:
	auto& songID() {
		return from<int>(this, 0x1a4);
	}
};

enum class CustomColorMode {
	Default = 0,
	PlayerCol1 = 1,
	PlayerCol2 = 2,
	LightBG = 5,
	Col1 = 3,
	Col2 = 4,
	Col3 = 6,
	Col4 = 7,
	DL = 8
};

class GameObject : public CCSprite {
public:
	auto& id() {
		return from<int>(this, 0x2c4);
	}
	auto& triggerColor() {
		return from<ccColor3B>(this, 0x2b8);
	}
	auto& triggerDuration() {
		return from<float>(this, 0x2bc);
	}
	auto& triggerBlending() {
		return from<bool>(this, 0x314);
	}
	auto& touchTriggered() {
		return from<bool>(this, 0x271);
	}
	void setObjectColor(ccColor3B color) {
		return reinterpret_cast<void(__thiscall*)(GameObject*, ccColor3B)>(base + 0x75560)(this, color);
	}
	bool getIsTintObject() const {
		return from<bool>(this, 0x2cb);
	}
	bool getHasColor() const {
		return from<bool>(this, 0x24a);
	}
	auto getChildSprite() {
		return from<CCSprite*>(this, 0x24c);
	}
	void setChildColor(ccColor3B color) {
		if (getHasColor()) {
			getChildSprite()->setColor(color);
		}
	}
	// my own func
	void setCustomColor(ccColor3B color) {
		if (getHasColor()) setChildColor(color);
		else setObjectColor(color);
	}
	auto getColorMode() {
		auto active = from<CustomColorMode>(this, 0x308);
		auto default_color = from<CustomColorMode>(this, 0x30c);
		// TODO: gd checks some boolean
		if (active == CustomColorMode::Default)
			active = default_color;
		return active;
	}
	bool isSelected() {
		return from<bool>(this, 0x316);
	}
	bool shouldBlendColor() {
		return reinterpret_cast<bool(__thiscall*)(GameObject*)>(base + 0x6ece0)(this);
	}
};

class FLAlertLayer : public CCLayerColor {
public:
	auto menu() {
		return from<CCMenu*>(this, 0x194);
	}
};

// inherits other classes too!
class ColorSelectPopup : public FLAlertLayer {
public:
	auto colorPicker() {
		return from<extension::CCControlColourPicker*>(this, 0x1c0);
	}
};

class CCMenuItemSpriteExtra : public CCMenuItemSprite {
public:
	static auto create(CCNode* sprite, CCNode* idk, CCObject* target, SEL_MenuHandler callback) {
		auto ret = reinterpret_cast<CCMenuItemSpriteExtra*(__fastcall*)(CCNode*, CCNode*, CCObject*, SEL_MenuHandler)>
			(base + 0xd1e0)(sprite, idk, target, callback);
		__asm add esp, 0x8;
		return ret;
	}
};

class ButtonSprite : public CCSprite {
public:
	static auto create(const char* label, int width, int idk, float scale, bool absolute, const char* font, const char* sprite, float height) {
		auto ret = reinterpret_cast<ButtonSprite*(__vectorcall*)(
			float, float, float, float, float, float, // xmm registers
			const char*, int, // ecx and edx
			int, bool, const char*, const char*, float // stack
		)>(base + 0x9800)(0.f, 0.f, 0.f, scale, 0.f, 0.f, label, width, idk, absolute, font, sprite, height);
		__asm add esp, 20;
		return ret;
	}
};

class EditorPauseLayer : public CCLayer {

};

class DrawGridLayer : public CCLayer {
public:
	float timeForXPos(float pos) {
		return reinterpret_cast<float(__vectorcall*)(
			float, float, float, float, float, float,
			DrawGridLayer*
		)>(base + 0x934f0)(0.f, pos, 0.f, 0.f, 0.f, 0.f, this);
	}
	auto getPlaybackLinePos() {
		return from<float>(this, 0x120);
	}
};

class SettingsColorObject : public CCNode {
public:
	ccColor3B m_color;
	bool m_blending;
	int m_custom;
};

static_assert(sizeof(SettingsColorObject) == 0xF0, "size wrong");

class LevelSettingsObject : public CCNode {
public:
	SettingsColorObject* m_background_color;
	SettingsColorObject* m_ground_color;
	SettingsColorObject* m_line_color;
	SettingsColorObject* m_object_color;
	SettingsColorObject* m_3dl_color;
	SettingsColorObject* m_color1;
	SettingsColorObject* m_color2;
	SettingsColorObject* m_color3;
	SettingsColorObject* m_color4;
};

class LevelEditorLayer : public CCLayer {
public:
	auto editorUI() {
		return from<EditorUI*>(this, 0x15c);
	}
	auto backgroundSprite() {
		return from<CCSprite*>(this, 0x160);
	}
	auto gameLayer() {
		return from<CCLayer*>(this, 0x188);
	}
	auto getLevelSettings() {
		return from<LevelSettingsObject*>(this, 0x190);
	}
	auto getDrawGrid() {
		return from<DrawGridLayer*>(this, 0x184);
	}
	auto getPlaytestState() {
		return from<int>(this, 0x198);
	}
	auto getPlayer1() {
		return from<PlayerObject*>(this, 0x19c);
	}
	auto getLevelSections() {
		return from<CCArray*>(this, 0x16c);
	}
	void addToSection(GameObject* object) {
		reinterpret_cast<void(__thiscall*)(LevelEditorLayer*, GameObject*)>(base + 0x8d220)(this, object);
	}
	auto getObjectBatchNode() {
		return from<CCSpriteBatchNode*>(this, 0x164);
	}
};

class LocalLevelManager : public CCNode {
public:
	static auto sharedState() {
		return reinterpret_cast<LocalLevelManager*(__stdcall*)()>(base + 0xac180)();
	}

	auto getLocalLevels() {
		return from<CCArray*>(this, 0x108);
	}
};

class LevelInfoLayer : public CCLayer {
public:
	auto level() {
		return from<GJGameLevel*>(this, 0x13c);
	}

	bool shouldDownloadLevel() {
		return reinterpret_cast<bool(__thiscall*)(LevelInfoLayer*)>(base + 0x9cc40)(this);
	}
};

class EditLevelLayer : public CCLayer {
public:
	auto level() {
		return from<GJGameLevel*>(this, 0x124);
	}
};