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

		explicit string(const std::string_view& str) : string(str.data(), str.size()) {}
		// string(const char* str) : string(std::string_view(str)) {}
		string(const std::string& str) : string(str.c_str(), str.size()) {}

		size_t size() const { return m_size; }

		string& operator=(const std::string& other) {
			if (m_capacity > 15) delete m_data.ptr;
			reinterpret_cast<void*(__thiscall*)(void*, const char*, size_t)>(base + 0xa3f0)(this, other.c_str(), other.size());
			return *this;
		}

		const char* c_str() const {
			if (m_capacity <= 15) return m_data.inline_data;
			return m_data.ptr;
		}

		std::string_view sv() const {
			return std::string_view(c_str(), m_size);
		}

		operator std::string() const { return std::string(sv()); }
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

enum class GJLevelType {
	Local = 1,
	Editor = 2,
	Saved = 3
};

class GJGameLevel : public CCNode {
public:
	auto& songID() {
		return from<int>(this, 0x1a4);
	}
	static auto create() {
		return reinterpret_cast<GJGameLevel*(__stdcall*)()>(base + 0x621c0)();
	}

	cocos2d::CCDictionary* m_lastBuildSave;
	int m_levelID;
	gd::string m_levelName;
	gd::string m_levelDesc;
	gd::string m_levelString;
	gd::string m_userName;
	gd::string m_recordString;
	gd::string m_uploadDate;
	gd::string m_updateDate;
	int m_userID;
	int m_difficulty;
	int m_audioTrack;
	int m_songID;
	int m_levelRev;
	int m_objectCount;
	int m_order;
	int m_ratings;
	int m_ratingsSum;
	int m_downloads;
	int m_completes;
	bool m_isEditable;
	bool m_isVerified;
	bool m_isUploaded;
	bool m_hasBeenModified;
	int m_levelVersion;
	int m_gameVersion;
	int m_attempts;
	int m_jumps;
	int m_normalPercent;
	int m_practicePercent;
	int m_likes;
	int m_dislikes;
	int m_levelLength;
	int m_featured;
	bool m_demon;
	int m_stars;
	bool m_autoLevel;
	int m_coins;
	int m_password;
	int m_originalLevel;
	bool m_twoPlayerMode;
	int m_failedPasswordAttempts;
	bool m_showedSongWarning;
	int m_starRatings;
	int m_starRatingsSum;
	int m_maxStarRatings;
	int m_minStarRatings;
	int m_demonVotes;
	int m_rateStars;
	bool m_rateFeature;
	gd::string m_rateUser;
	bool m_dontSave;
	bool m_isHidden;
	int m_requiredCoins;
	bool m_isUnlocked;
	cocos2d::CCPoint m_lastCameraPos;
	float m_lastEditorZoom;
	int m_lastBuildTab;
	int m_lastBuildPage;
	int m_lastBuildGroupID;
	GJLevelType m_levelType;
	int m_ID;
	gd::string m_tempName;
	int m_capacity001;
	int m_capacity002;
	int m_capacity003;
	int m_capacity004;
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

class FLAlertLayerProtocol {

};

class FLAlertLayer : public CCLayerColor {
public:
	auto menu() {
		return from<CCMenu*>(this, 0x194);
	}

	static auto create(FLAlertLayerProtocol* protocol, const char* title,
		const char* caption, const char* button1, const char* button2,
		float height, bool absolute, float width) {
		auto ret = reinterpret_cast<FLAlertLayer*(__fastcall*)(FLAlertLayerProtocol*, const char*, const char*,
			const char*, const char*, float, bool, float)>(base + 0x15360)(protocol, title,
			caption, button1, button2, height, absolute, width);
		__asm sub esp, 4
		return ret;
	}

	// void show() {
	// 	return reinterpret_cast<void(__thiscall*)(FLAlertLayer*)>(base + 0x160a0)(this);
	// }
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
		// TODO: fix vectorcall version to work with clang
		// __asm movss xmm3, scale
		// auto ret = reinterpret_cast<ButtonSprite*(__fastcall*)(
		// 	const char*, int, // ecx and edx
		// 	int, bool, const char*, const char*, float // stack
		// )>(base + 0x9800)(label, width, idk, absolute, font, sprite, height);
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
	static auto scene(GJGameLevel* level) {
		return reinterpret_cast<CCScene*(__fastcall*)(GJGameLevel*)>(base + 0x3b4c0)(level);
	} 
};

class GameLevelManager : public CCNode {
public:
	static auto sharedState() {
		return reinterpret_cast<GameLevelManager*(__stdcall*)()>(base + 0x55850)();
	}

	GJGameLevel* createNewLevel() {
		return reinterpret_cast<GJGameLevel*(__thiscall*)(GameLevelManager*)>(base + 0x56590)(this);
	}
};

enum class SearchType {
	Search = 0,
	Downloaded = 1,
	MostLiked = 2,
	Trending = 3,
	Recent = 4,
	UsersLevels = 5,
	Featured = 6,
	Magic = 7,
	Sends = 8,
	MapPack = 9,
	MapPackOnClick = 10,
	Awarded = 11,
	Followed = 12,
	Friends = 13,
	Users = 14,
	LikedGDW = 15,
	HallOfFame = 16,
	FeaturedGDW = 17,
	Similar = 18,
	MyLevels = 98,
	SavedLevels = 99,
	FavouriteLevels = 100
};

class GJSearchObject : public CCNode {
public:
	static auto create(SearchType type, const std::string& query, const std::string& difficultyStr,
		const std::string& lengthStr, int page, bool isStar, bool isCompleted,
		bool isFeatured, int song, bool isOriginal, bool isTwoP,
		bool isCustomSong, bool isSongFilter, bool isNoStar) {
		return reinterpret_cast<GJSearchObject*(__fastcall*)(SearchType, gd::string, gd::string,
			gd::string, int, bool, bool,
			bool, int, bool, bool,
			bool, bool, bool)>(base + 0x652a0)(
			type, query, difficultyStr, lengthStr, page, isStar,
			isCompleted, isFeatured, song, isOriginal, isTwoP,
			isCustomSong, isSongFilter, isNoStar
		);
	}
	static auto create(SearchType type) {
		return reinterpret_cast<GJSearchObject*(__fastcall*)(SearchType)>(base + 0x650d0)(type);
		// return create(type, "", "-", "-", 0, false, false, false, 0, false, false, false, false, false);
	}

	SearchType m_type;
	gd::string m_string;
	gd::string m_difficultyStr;
	gd::string m_lengthStr;
	int m_page;
	bool m_star;
	bool m_noStar;
	int m_total;
	bool m_uncompleted;
	bool m_featured;
	bool m_original;
	bool m_twoPlayer;
	int m_song;
	bool m_customSong;
	bool m_songFilter;
};

class LevelBrowserLayer : public CCLayer {
public:
	static auto create(GJSearchObject* obj) {
		return reinterpret_cast<LevelBrowserLayer*(__fastcall*)(GJSearchObject*)>(base + 0x894f0)(obj);
	}
};

class CustomizeObjectLayer : public FLAlertLayer {
public:
	void hightlightSelected(ButtonSprite* spr) {
		reinterpret_cast<void(__thiscall*)(CustomizeObjectLayer*, ButtonSprite*)>(base + 0x2e730)(this, spr);
	}
};