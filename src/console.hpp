#pragma once
#include <fstream>
#include <iostream>
#include <functional>

struct Console {
	std::ofstream out, in;
	void setup() {
		AllocConsole();
		out = decltype(out)("CONOUT$", std::ios::out);
		in = decltype(in)("CONIN$", std::ios::in);
		std::cout.rdbuf(out.rdbuf());
		std::cin.rdbuf(in.rdbuf());
	}
	~Console() {
		out.close();
		in.close();
	}
};

template <class... Args>
void log(const std::string_view& str, Args&&... args) {
	if constexpr (sizeof...(Args) == 0) {
		for (size_t i = 0; i < str.size(); ++i) {
			const auto c = str[i];
			if (c == '\n') std::cout << std::endl;
			else std::cout << c;
		}
	} else {
		const std::function<void()> partials[sizeof...(Args)] = { [&]() { std::cout << args; }... };
		size_t counter = 0;
		for (size_t i = 0; i < str.size(); ++i) {
			const auto c = str[i];
			if (c == '{' && str[i + 1] == '}') (partials[counter++](), ++i);
			else if (c == '\n') std::cout << std::endl;
			else std::cout << c;
		}
	}
}
