#define NOMINMAX
#include "preview-mode.hpp"
#include "state.hpp"
#include "gd.hpp"
#include <matdash.hpp>
#include <matdash/minhook.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "utils.hpp"
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <unordered_set>

using namespace matdash;

struct CompareTriggers {
	bool operator()(GameObject* a, GameObject* b) const {
		return a->getPosition().x < b->getPosition().x;
	}

	bool operator()(GameObject* a, float b) const {
		return a->getPosition().x < b;
	}

	bool operator()(float a, GameObject* b) const {
		return a < b->getPosition().x;
	}
};

template <class T = void*>
struct ValueContainer {
	T value;
	virtual ~ValueContainer() {
		// std::cout << "value container destroyed type " << typeid(T).name() << std::endl;
		value.~T();
	}
	template <class A>
	A& get() { return reinterpret_cast<ValueContainer<A>*>(this)->value; }
};

template <class T>
struct Field {};

template <class T>
struct ExtendBase {
	static auto& get_extra_fields() {
		static std::unordered_map<void*, ValueContainer<>*> fields;
		return fields;
	}

	static void destroy() {
		auto& fields = get_extra_fields();
		for (auto& [_, value] : fields) {
			delete value;
		}
		fields.clear();
	}
};

template <class T, class A>
A& operator->*(ExtendBase<T>* self, Field<A>& dummy) {
	auto& fields = ExtendBase<T>::get_extra_fields();
	void* key = reinterpret_cast<void*>(&dummy);
	auto it = fields.find(key);
	if (it != fields.end()) return it->second->get<A>();
	auto container = reinterpret_cast<ValueContainer<>*>(new ValueContainer<A>());
	fields[key] = container;
	return container->get<A>();
}

bool is_editor_paused = false;

template <class T>
T lerp(float amt, T a, T b) {
	return static_cast<T>(static_cast<float>(a) * (1.f - amt) + static_cast<float>(b) * amt);
}

struct GDColor {
	u8 r, g, b;
	bool blending = false;

	GDColor() {}
	GDColor(u8 r, u8 g, u8 b, bool blending) : r(r), g(g), b(b), blending(blending) {}
	GDColor(const ccColor3B color, bool blending = false) : r(color.r), g(color.g), b(color.b), blending(blending) {}
	GDColor(GameObject* object) : GDColor(object->triggerColor(), object->triggerBlending()) {}
	GDColor(SettingsColorObject* color) : GDColor(color->m_color, color->m_blending) {}
	operator ccColor3B() const { return { r, g, b }; }
};

GDColor mix_color(float value, GDColor a, GDColor b) {
	return GDColor {
		lerp<u8>(value, a.r, b.r),
		lerp<u8>(value, a.g, b.g),
		lerp<u8>(value, a.b, b.b),
		b.blending
	};
}

enum class ColorTriggers {
	BG = 29,
	Obj = 105,
	// 3DL
	DLine = 744,
	Col1 = 221,
	Col2 = 717,
	Col3 = 718,
	Col4 = 743,
};

static std::unordered_set<int> color_trigger_ids = { 29, 105, 221, 717, 718, 743, 744 };
bool is_color_trigger(GameObject* object) {
	return color_trigger_ids.find(object->id()) != color_trigger_ids.end();
}

void log_obj_vector(const std::vector<GameObject*>& objects) {
	std::cout << '[';
	for (auto obj : objects)
		std::cout << "id=" << obj << " x=" << obj->getPosition().x << ", ";
	std::cout << ']' << std::endl;
}

// TODO: this
GDColor calculate_lbg(const GDColor& bg_color) {
	auto color = bg_color;
	color.blending = true;
	return color;
}

class MyEditorLayer : public LevelEditorLayer, public ExtendBase<MyEditorLayer> {
public:
	Field<std::unordered_map<ColorTriggers, std::vector<GameObject*>>> m_color_triggers;
	Field<std::unordered_map<CustomColorMode, bool>> m_current_color;
	Field<std::unordered_set<GameObject*>> m_blending_objs;
	Field<float> m_last_pos;
	Field<CCSpriteBatchNode*> m_blending_batch_node;
	static MyEditorLayer* s_instance;

	void dtor() {
		ExtendBase::destroy();
		orig<&MyEditorLayer::dtor>(this);
		s_instance = nullptr;
	}

	bool init(GJGameLevel* level) {
		if (!orig<&MyEditorLayer::init>(this, level)) return false;
		s_instance = this;

		auto& triggers = this->*m_color_triggers;
		// initialize them with empty vectors
		triggers[ColorTriggers::BG];
		triggers[ColorTriggers::Obj];
		triggers[ColorTriggers::DLine];
		triggers[ColorTriggers::Col1];
		triggers[ColorTriggers::Col2];
		triggers[ColorTriggers::Col3];
		triggers[ColorTriggers::Col4];

		auto gamesheet_texture = CCTextureCache::sharedTextureCache()->addImage("GJ_GameSheet.png", false);
		auto blending_batch_node = CCSpriteBatchNode::createWithTexture(gamesheet_texture);
		println("blending node {}", blending_batch_node);
		this->gameLayer()->addChild(blending_batch_node, 0);

		blending_batch_node->setBlendFunc({ GL_SRC_ALPHA, GL_ONE });

		this->*m_blending_batch_node = blending_batch_node;

		return true;
	}

	void insert_trigger(GameObject* object) {
		auto& triggers = (this->*m_color_triggers)[ColorTriggers(object->id())];
		for (size_t i = 0; i < triggers.size(); ++i) {
			if (CompareTriggers()(object, triggers[i])) {
				triggers.insert(triggers.begin() + i, object);
				return;
			}
		}
		triggers.push_back(object);
	}

	void remove_trigger(GameObject* object) {
		auto& triggers = (this->*m_color_triggers)[ColorTriggers(object->id())];
		for (size_t i = 0; i < triggers.size(); ++i) {
			if (triggers[i] == object) {
				triggers.erase(triggers.begin() + i);
				break;
			}
		}
	}

	void move_trigger(GameObject* object) {
		if (is_color_trigger(object)) {
			// TODO: just swap it instead of removing and reinserting
			this->remove_trigger(object);
			this->insert_trigger(object);
		}
	}

	float time_between_pos(float a, float b) {
		auto l = this->getDrawGrid();
		return std::abs(l->timeForXPos(a) - l->timeForXPos(b));
	}

	GDColor calculate_color(std::vector<GameObject*> triggers, const float pos, const GDColor starting_color) {
		if (triggers.empty()) return starting_color;
		auto bound = std::lower_bound(triggers.begin(), triggers.end(), pos, CompareTriggers()) - triggers.begin();
		if (bound == 0) {
			return starting_color;
		} else {
			auto trigger = triggers[bound - 1];
			GDColor color_to = trigger;
			auto dist = time_between_pos(trigger->getPosition().x, pos) / trigger->triggerDuration();
			auto color_from = starting_color;
			if (bound > 1) {
				auto trigger = triggers[bound - 2];
				auto dist = time_between_pos(trigger->getPosition().x, pos) / trigger->triggerDuration();
				color_from = trigger;
				// TODO: maybe do this recursively?
				if (dist < 1.f)
					color_from = mix_color(dist, bound > 2 ? triggers[bound - 3] : starting_color, color_from);
			}
			return mix_color(std::min(dist, 1.f), color_from, color_to);
		}
	}

	void addSpecial(GameObject* object) {
		orig<&MyEditorLayer::addSpecial>(this, object);
		if (is_color_trigger(object)) {
			this->insert_trigger(object);
		}
	}

	void removeSpecial(GameObject* object) {
		orig<&MyEditorLayer::removeSpecial>(this, object);
		if (is_color_trigger(object)) {
			this->remove_trigger(object);
		}
	}

	void update_object_color(GameObject* object, const GDColor& color) {
		object->setCustomColor(color);
		CCSprite* node = object;
		if (object->getHasColor())
			node = object->getChildSprite();
		auto batch = this->*m_blending_batch_node;
		// TODO: proper z ordering (copy from zmx)
		if (color.blending) {
			if (node->getParent() != batch) {
				node->removeFromParent();
				batch->addChild(node);
			}
		} else {
			if (node->getParent() == batch) {
				node->removeFromParent();
				this->getObjectBatchNode()->addChild(node);
			}
		}
	}

	bool is_color_blending(CustomColorMode mode) {
		return (this->*m_current_color)[mode];
	}

	void updateVisibility(float dt) {
		orig<&MyEditorLayer::updateVisibility, Thiscall>(this, dt);

		if (is_editor_paused || !state().preview_mode) return;

		float pos;
		if (this->getPlaytestState() != 0)
			pos = this->getPlayer1()->getPositionX();
		else if (this->editorUI()->isPlayback())
			pos = this->getDrawGrid()->getPlaybackLinePos();
		else
			pos = -(this->gameLayer()->getPosition().x) / this->gameLayer()->getScale() + 285;

		if (std::abs(pos - this->*m_last_pos) < 0.01f) return;
		this->*m_last_pos = pos;
		
		GDColor bg_color, obj_color, dl_color, color1, color2, color3, color4;
	
		auto settings = this->getLevelSettings();
		for (auto& [type, triggers] : this->*m_color_triggers) {
			GDColor* color = nullptr;
			GDColor starting_color;
			switch (type) {
			case ColorTriggers::BG:
				starting_color = settings->m_background_color;
				color = &bg_color;
				break;
			case ColorTriggers::Obj:
				starting_color = settings->m_object_color;
				color = &obj_color;
				break;
			case ColorTriggers::DLine:
				starting_color = settings->m_3dl_color;
				color = &dl_color;
				break;
			case ColorTriggers::Col1:
				starting_color = settings->m_color1;
				color = &color1;
				break;
			case ColorTriggers::Col2:
				starting_color = settings->m_color2;
				color = &color2;
				break;
			case ColorTriggers::Col3:
				starting_color = settings->m_color3;
				color = &color3;
				break;
			case ColorTriggers::Col4:
				starting_color = settings->m_color4;
				color = &color4;
				break;
			default:
				continue;
			}
			*color = this->calculate_color(triggers, pos, starting_color);
		}

		(this->*m_current_color)[CustomColorMode::Col1] = color1.blending;
		(this->*m_current_color)[CustomColorMode::Col2] = color2.blending;
		(this->*m_current_color)[CustomColorMode::Col3] = color3.blending;
		(this->*m_current_color)[CustomColorMode::Col4] = color4.blending;

		this->backgroundSprite()->setColor(bg_color);

		auto lbg_color = calculate_lbg(bg_color);
		// TODO: pcol1 and pcol2

		auto sections = AwesomeArray<CCArray>(this->getLevelSections());
		for (auto objects : sections) {
			if (!objects) continue;
			for (auto object : AwesomeArray<GameObject>(objects)) {
				if (!object || !object->getParent() || object->isSelected()) continue;

				if (object->getIsTintObject())
					object->setObjectColor(obj_color);

				auto mode = object->getColorMode();
				switch (mode) {
				case CustomColorMode::DL:
					this->update_object_color(object, dl_color);
					break;
				case CustomColorMode::Col1:
					this->update_object_color(object, color1);
					break;
				case CustomColorMode::Col2:
					this->update_object_color(object, color2);
					break;
				case CustomColorMode::Col3:
					this->update_object_color(object, color3);
					break;
				case CustomColorMode::Col4:
					this->update_object_color(object, color4);
					break;
				case CustomColorMode::LightBG:
					this->update_object_color(object, lbg_color);
					break;
				}
			}
		}
	}
};

MyEditorLayer* MyEditorLayer::s_instance = nullptr;

bool EditorPauseLayer_init(void* self, void* idc) {
	is_editor_paused = true;
	return orig<&EditorPauseLayer_init>(self, idc);
}

void EditorPauseLayer_dtor(void* self) {
	is_editor_paused = false;
	orig<&EditorPauseLayer_dtor>(self);
}

bool GameObject_shouldBlendColor(GameObject* self) {
	if (GameManager::sharedState()->getPlayLayer())
		return orig<&GameObject_shouldBlendColor>(self);
	else {
		auto editor = MyEditorLayer::s_instance;
		if (!editor) return false;

		switch (self->getColorMode()) {
		case CustomColorMode::Col1:
		case CustomColorMode::Col2:
		case CustomColorMode::Col3:
		case CustomColorMode::Col4:
			return editor->is_color_blending(self->getColorMode());
		}
		return false;
	}
}

void EditorUI_moveObject(EditorUI* self, GameObject* object, CCPoint to) {
	orig<&EditorUI_moveObject>(self, object, to);
	reinterpret_cast<MyEditorLayer*>(self->getParent())->move_trigger(object);
}

void EditorUI_scrollWheel(EditorUI* _self, float dy, float dx) {
	auto self = reinterpret_cast<EditorUI*>(reinterpret_cast<uintptr_t>(_self) - 0xf8);
	auto layer = reinterpret_cast<LevelEditorLayer*>(self->getParent())->gameLayer();
	auto zoom = layer->getScale();

	static_assert(offsetof(CCDirector, m_pKeyboardDispatcher) == 0x4c, "it wrong!");
	auto kb = CCDirector::sharedDirector()->m_pKeyboardDispatcher;
	if (kb->getControlKeyPressed()) {
		zoom = static_cast<float>(std::pow(2.71828182845904523536, std::log(std::max(zoom, 0.001f)) - dy * 0.01f));
		// zoom limit
		zoom = std::max(zoom, 0.01f);
		zoom = std::min(zoom, 1000000.f);
		self->updateZoom(zoom);
	} else if (kb->getShiftKeyPressed()) {
		layer->setPositionX(layer->getPositionX() - dy * 1.f);
	} else {
		orig<&EditorUI_scrollWheel, Thiscall>(_self, dy, dx);
	}
}

void preview_mode::init() {
	add_hook<&MyEditorLayer::updateVisibility, Thiscall>(base + 0x8ef20);
	add_hook<&MyEditorLayer::addSpecial>(base + 0x8ed10);
	add_hook<&MyEditorLayer::init>(base + 0x8c2c0);
	add_hook<&MyEditorLayer::dtor>(base + 0x8c080);
	add_hook<&EditorUI_moveObject>(base + 0x4b410);
	add_hook<&MyEditorLayer::removeSpecial>(base + 0x8ee30);

	add_hook<&EditorUI_scrollWheel, Thiscall>(base + 0x4ee90);

	add_hook<&EditorPauseLayer_init>(base + 0x3e2e0);
	add_hook<&EditorPauseLayer_dtor>(base + 0x3e280);

	add_hook<&GameObject_shouldBlendColor>(base + 0x6ece0);
}