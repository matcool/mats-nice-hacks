#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cocos2d.h>
#include <matdash.hpp>
#include <sstream>
#include <functional>
#include <string_view>
#include <filesystem>

namespace {
	template <class F>
	struct ThiscallWrapper;
	
	template <class R, class... Args>
	struct ThiscallWrapper<R(*)(Args...)> {
		template <auto func>
		static R __thiscall wrap(Args... args) {
			return func(args...);
		}
	};

	template <typename T>
	using __to_handler_f_type = typename RemoveThiscall<typename MemberToFn<T>::type>::type;
}

template <typename T, typename U>
T union_cast(U value) {
	static_assert(sizeof(T) == sizeof(U), "union_cast sizes must mach");
	union {
		T a;
		U b;
	} u;
	u.b = value;
	return u.a;
}

template <typename H, __to_handler_f_type<H> Func>
static const auto to_handler = union_cast<H>(ThiscallWrapper<decltype(Func)>::wrap<Func>);

inline bool operator==(const cocos2d::ccColor3B a, const cocos2d::ccColor3B b) { return a.r == b.r && a.g == b.g && a.b == b.b; }
inline bool operator!=(const cocos2d::ccColor3B a, const cocos2d::ccColor3B b) { return a.r != b.r || a.g != b.g || a.b != b.b; }

struct Color3F {
	float r, g, b;
	static Color3F from(const cocos2d::ccColor3B color) {
		return { float(color.r) / 255.f, float(color.g) / 255.f, float(color.b) / 255.f };
	}
	operator cocos2d::ccColor3B() const {
		return { uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255) };
	}
};

inline bool operator!=(const cocos2d::CCPoint& a, const cocos2d::CCPoint& b) { return a.x != b.x || a.y != b.y; }

template <class... Args>
std::string format(const std::string_view& str, Args&&... args) {
	if constexpr (sizeof...(Args) == 0) {
		return str; // why would u do this
	} else {
		std::stringstream stream;
		const std::function<void(std::stringstream&)> partials[sizeof...(Args)] = { [args](std::stringstream& stream) { stream << args; }... };
		size_t counter = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			const auto c = str[i];
			if (c == '{' && str[i + 1] == '}') (partials[counter++](stream), ++i);
			else stream << c;
		}
		return stream.str();
	}
}

inline auto get_exe_path() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(NULL), buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
}

inline std::pair<std::string, std::string> split_once(const std::string& str, char split) {
	const auto n = str.find(split);
	return { str.substr(0, n), str.substr(n + 1) };
}

inline void patch(uintptr_t addr, const std::vector<uint8_t>& bytes) {
	DWORD old_prot;
	VirtualProtect(reinterpret_cast<void*>(addr), bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot);
	memcpy(reinterpret_cast<void*>(addr), bytes.data(), bytes.size());
	VirtualProtect(reinterpret_cast<void*>(addr), bytes.size(), old_prot, &old_prot);
}

template <auto F>
auto cocos_symbol(const char* name) {
	static const auto addr = GetProcAddress((HMODULE)cocos_base, name);
	return reinterpret_cast<MemberToFn<decltype(F)>::type>(addr);
}