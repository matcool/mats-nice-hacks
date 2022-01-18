#pragma once

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
};

class AppDelegate : public CCApplication {

};

class GJGameLevel : public CCNode {
public:
	auto& songID() {
		return from<int>(this, 0x1a4);
	}
};

class GameObject : public CCSprite {

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

class LevelEditorLayer : public CCLayer {
public:
	auto editorUI() {
		return from<EditorUI*>(this, 0x15c);
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