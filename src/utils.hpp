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
#include <stdint.h>

using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;


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

template <class Stream, class... Args>
void format_to(Stream& stream, const std::string_view& str, Args&&... args) {
	if constexpr (sizeof...(Args) == 0) {
		stream << str;
	} else {
		const std::function<void(Stream&)> partials[sizeof...(Args)] = { [args](Stream& stream) { stream << args; }... };
		size_t counter = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			const auto c = str[i];
			if (c == '{' && str[i + 1] == '}') (partials[counter++](stream), ++i);
			else stream << c;
		}
	}
}

template <class... Args>
std::string format(const std::string_view& str, Args&&... args) {
	std::stringstream stream;
	format_to(stream, str, args...);
	return stream.str();
}

template <class... Args>
void println(const std::string_view& str, Args&&... args) {
	format_to(std::cout, str, args...);
	std::cout << std::endl;
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

template <typename T>
struct CCArrayIterator {
public:
    CCArrayIterator(T* p) : m_ptr(p) {}
    T* m_ptr;

    T& operator*() { return *m_ptr; }
    T* operator->() { return m_ptr; }

    auto& operator++() {
        ++m_ptr;
        return *this;
    }

    friend bool operator== (const CCArrayIterator<T>& a, const CCArrayIterator<T>& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const CCArrayIterator<T>& a, const CCArrayIterator<T>& b) { return a.m_ptr != b.m_ptr; };   
};

template <typename T>
class AwesomeArray {
public:
    AwesomeArray(cocos2d::CCArray* arr) : m_arr(arr) {}
    cocos2d::CCArray* m_arr;
    auto begin() { return CCArrayIterator<T*>(reinterpret_cast<T**>(m_arr->data->arr)); }
    auto end() { return CCArrayIterator<T*>(reinterpret_cast<T**>(m_arr->data->arr) + m_arr->count()); }

    auto size() const { return m_arr->count(); }
    T* operator[](size_t index) { return reinterpret_cast<T*>(m_arr->objectAtIndex(index)); }
};

template <typename K, typename T>
struct CCDictIterator {
public:
    CCDictIterator(cocos2d::CCDictElement* p) : m_ptr(p) {}
    cocos2d::CCDictElement* m_ptr;

    std::pair<K, T> operator*() {
        if constexpr (std::is_same<K, std::string>::value) {
            return { m_ptr->getStrKey(), reinterpret_cast<T>(m_ptr->getObject()) };
        } else {
            return { m_ptr->getIntKey(), reinterpret_cast<T>(m_ptr->getObject()) };
        }
    }

    auto& operator++() {
        m_ptr = reinterpret_cast<decltype(m_ptr)>(m_ptr->hh.next);
        return *this;
    }

    friend bool operator== (const CCDictIterator<K, T>& a, const CCDictIterator<K, T>& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const CCDictIterator<K, T>& a, const CCDictIterator<K, T>& b) { return a.m_ptr != b.m_ptr; };
    bool operator!= (int b) { return m_ptr != nullptr; }
};


template <typename K, typename T>
struct AwesomeDict {
public:
    AwesomeDict(cocos2d::CCDictionary* dict) : m_dict(dict) {}
    cocos2d::CCDictionary* m_dict;
    auto begin() { return CCDictIterator<K, T*>(m_dict->m_pElements); }
    // do not use this
    auto end() { return nullptr; }

    auto size() { return m_dict->count(); }
    T* operator[](K key) { return reinterpret_cast<T*>(m_dict->objectForKey(key)); }
};