#pragma once
#include <type_traits>
#include <tuple>
#include <utility>

namespace meta {
	namespace {
		template <class T>
		using float_default = std::conditional_t<std::is_floating_point_v<T>, T, float>;

		template <class T>
		static constexpr bool is_reg = !std::is_class_v<T> && !std::is_floating_point_v<T> && (sizeof(T) <= 4);

		template <class T>
		using reg_default = std::conditional_t<is_reg<T>, T, int>;

		template <bool V>
		struct ternary_v {
			template <class T, class U>
			constexpr auto value(T&& a, U&& b) {
				return std::forward<T>(a);
			}
		};

		template <>
		struct ternary_v<false> {
			template <class T, class U>
			constexpr auto value(T&& a, U&& b) {
				return std::forward<U>(b);
			}
		};

		template <size_t i, size_t... indexes>
		struct true_make_true_rest {
			template <class T, class... Args>
			auto work(T&& value, Args&&... rest) {
				if constexpr (sizeof...(indexes) == 0) {
					if constexpr (i > 3) return std::make_tuple(value);
					else return std::make_tuple();
				} else {
					if constexpr (i > 3) return std::tuple_cat(std::make_tuple(value), true_make_true_rest<indexes...>::work(rest...));
					else return true_make_true_rest<indexes...>::work(rest...);
				}
			}
		};

		template <size_t... indexes>
		struct make_true_rest {
			template <class... Args>
			auto work(Args&&... args) {
				if constexpr (sizeof...(indexes) == 0) return std::make_tuple();
				else {
					return true_make_true_rest<indexes...>::work<Args...>(args...);
				}
			}	
		};
	}

	template <class R, class... Args>
	struct OptcallWrapper {
		using types = std::tuple<Args...>;
 
		template <auto func, size_t... indexes>
		R __vectorcall wrap(
			float_default<std::tuple_element_t<0, types>> xmm0,
			float_default<std::tuple_element_t<1, types>> xmm1,
			float_default<std::tuple_element_t<2, types>> xmm2,
			float_default<std::tuple_element_t<3, types>> xmm3,
			float xmm4, float xmm5,
			reg_default<std::tuple_element_t<0, types>> ecx,
			reg_default<std::tuple_element_t<1, types>> edx,
			std::tuple_element_t<indexes, types>... rest
		) {
			auto& stack_values = std::make_tuple(rest...);
			auto& true_rest = make_true_rest<indexes...>::work(rest...);
			return func(
				ternary_v<std::is_floating_point_v<std::tuple_element_t<0, types>>>::value(
					xmm0, ternary_v<is_reg<std::tuple_element_t<0, types>>>::value(
						ecx, std::get<0>(stack_values))),
				ternary_v<std::is_floating_point_v<std::tuple_element_t<1, types>>>::value(
					xmm1, ternary_v<is_reg<std::tuple_element_t<1, types>>>::value(
						edx, std::get<1>(stack_values))),
				ternary_v<std::is_floating_point_v<std::tuple_element_t<2, types>>>::value(
					xmm2, std::get<2>(stack_values)),
				ternary_v<std::is_floating_point_v<std::tuple_element_t<3, types>>>::value(
					xmm3, std::get<3>(stack_values)),
				true_rest...
			);
		}

	};
}