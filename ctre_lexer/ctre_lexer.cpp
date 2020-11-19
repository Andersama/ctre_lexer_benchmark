// ctre_lexer.cpp : Defines the entry point for the application.
//

#include "nanobench.h"

#include "ctre.hpp"
#include <experimental/vector>
#include <string_view>
#include <iostream>
#include <array>

#if defined(__clang__) || defined(__GNUC__)
#include <x86intrin.h>
#elif  _MSC_VER
#include <intrin.h>
#endif

using namespace std;

static constexpr auto spacing_patterns = ctll::fixed_string(
	R"RGX([\s]+|[\S]+)RGX"
);

static constexpr auto overlap_strs = ctll::fixed_string(
	R"RGX((?:ab|abc|abcd)|[\w\W])RGX"
);

std::string_view rando_test_code = R"CODE(
	//a comment
	/*
	a multiline comment
	*/
	/+
	/+
	a nested comment
	+/
	+/
	struct somestruct {
		void test(param name, param2 name2){
			return 0;
		}
	};
	int main(void){
		for (size_t i = 0; i < 99; i++) {
			__noop;
		}
		return 0;
	}
)CODE";

int main()
{
	std::vector<std::string_view> tokens;
	std::vector<std::string_view> tokens2;
	std::vector<std::string_view> tokens3;
	std::vector<std::string_view> tokens4;

	ankerl::nanobench::Bench benchmark;
	benchmark.epochs(1024);
	benchmark.minEpochIterations(128);
	benchmark.minEpochTime(std::chrono::nanoseconds{ 4000 });
	benchmark.warmup(4);
	//for metrics
	benchmark.unit("byte");
	benchmark.performanceCounters(true);
	benchmark.relative(true);

	benchmark.batch(rando_test_code.size());

	alignas(32) constexpr auto custom_class_table = []() {
		std::array<uint8_t, 256> dfa_table{};
		for (size_t i = 0; i < dfa_table.size(); i++) {
			dfa_table[i] = ctre::space_chars::match_char((uint8_t)i);
		}
		return dfa_table;
	}();

	alignas(32) constexpr auto custom_table = []() {
		std::array<std::array<uint8_t, 2>, 3> dfa_table{};
		dfa_table[0][0] = 1; //non wspace state
		dfa_table[0][1] = 2; //wpsace state

		dfa_table[1][0] = 1; //keep in loop state etc...
		dfa_table[1][1] = 3;

		dfa_table[2][0] = 4;
		dfa_table[2][1] = 2;

		return dfa_table;
	}();

	alignas(32) constexpr auto custom_table_sse = []() {
		std::array<std::array<uint8_t, 16>, 3> dfa_table{};
		dfa_table[0][0] = 1; //non wspace state
		dfa_table[0][1] = 2; //wpsace state

		dfa_table[1][0] = 1; //keep in loop state etc...
		dfa_table[1][1] = 3;

		dfa_table[2][0] = 4;
		dfa_table[2][1] = 2;

		return dfa_table;
	}();
	
	benchmark.run("hand_coded table", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens2.clear();
		uint32_t state = 0;
		for (; idx < rando_test_code.size();) {
			uint8_t chr = rando_test_code[idx];
			uint8_t eqc = custom_class_table[chr];
			state = custom_table[state][eqc];
			if (state > 2) [[unlikely]] {
				tokens2.emplace_back(rando_test_code.substr(prev, idx - prev));
				prev = idx;
				state = 0;
			}
			idx++;
		}
	});

	benchmark.run("hand_coded table sse", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens2.clear();
		uint32_t state = 0;
		__m128i states = _mm_set1_epi8(0);
		for (; (idx+7) < rando_test_code.size();) {
			uint8_t c1 = rando_test_code[idx];
			uint8_t e1 = custom_class_table[c1];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e1], states);
			uint8_t state1 = _mm_cvtsi128_si32(states);

			uint8_t c2 = rando_test_code[idx + 1];
			uint8_t e2 = custom_class_table[c2];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e2], states);
			uint8_t state2 = _mm_cvtsi128_si32(states);

			uint8_t c3 = rando_test_code[idx + 2];
			uint8_t e3 = custom_class_table[c3];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e3], states);
			uint8_t state3 = _mm_cvtsi128_si32(states);

			uint8_t c4 = rando_test_code[idx + 3];
			uint8_t e4 = custom_class_table[c4];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e4], states);
			uint8_t state4 = _mm_cvtsi128_si32(states);

			uint8_t c5 = rando_test_code[idx + 4];
			uint8_t e5 = custom_class_table[c5];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e5], states);
			uint8_t state5 = _mm_cvtsi128_si32(states);

			uint8_t c6 = rando_test_code[idx + 5];
			uint8_t e6 = custom_class_table[c6];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e6], states);
			uint8_t state6 = _mm_cvtsi128_si32(states);

			uint8_t c7 = rando_test_code[idx + 6];
			uint8_t e7 = custom_class_table[c7];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e7], states);
			uint8_t state7 = _mm_cvtsi128_si32(states);

			uint8_t c8 = rando_test_code[idx + 7];
			uint8_t e8 = custom_class_table[c8];
			states = _mm_shuffle_epi8(*(__m128i*) & custom_table_sse[e8], states);
			uint8_t state8 = _mm_cvtsi128_si32(states);

			if (state1 > 2 ||
				state2 > 2 ||
				state3 > 2 ||
				state4 > 2 ||
				state5 > 2 ||
				state6 > 2 ||
				state7 > 2 ||
				state8 > 2
				) [[unlikely]] {
				uint32_t offset = 0;
				if (state1 > 2)
					offset = 0;
				else if (state2 > 2)
					offset = 1;
				else if (state3 > 2)
					offset = 2;
				else if (state4 > 2)
					offset = 3;
				else if (state5 > 2)
					offset = 4;
				else if (state6 > 2)
					offset = 5;
				else if (state7 > 2)
					offset = 6;
				else
					offset = 7;
				tokens2.emplace_back(rando_test_code.substr(prev, (idx+offset) - prev));
				prev = idx;
				states = _mm_set1_epi8(0);
			}
			state = state8;
			idx += 8;
		}

		for (; idx < rando_test_code.size();) {
			uint8_t chr = rando_test_code[idx];
			uint8_t eqc = custom_class_table[chr];
			state = custom_table[state][eqc];
			if (state > 2) [[unlikely]] {
				tokens2.emplace_back(rando_test_code.substr(prev, idx - prev));
				prev = idx;
				state = 0;
			}
			idx++;
		}

	});

	//a look-alike to what I originally wrote
	benchmark.run("hand_coded switch inside if", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens3.clear();
		uint32_t state = 0;
		while (idx < rando_test_code.size()) {
			for (; idx < rando_test_code.size();) {
				uint8_t chr = rando_test_code[idx];
				uint8_t eqc = custom_class_table[chr];
				state = custom_table[state][eqc];
				if (state > 2) [[unlikely]] {
					switch (state)
					{
					case 3:
					case 4:
						tokens3.emplace_back(rando_test_code.substr(prev, idx - prev));
						break;
					default:
						break;
					}
					prev = idx;
					state = 0;
				}
				idx++;
			}
		}
	});

	benchmark.run("hand_coded switch only in loop", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens3.clear();
		uint32_t state = 0;
		while (idx < rando_test_code.size()) {
			for (; idx < rando_test_code.size();) {
				uint8_t chr = rando_test_code[idx];
				uint8_t eqc = custom_class_table[chr];
				state = custom_table[state][eqc];
				switch (state)
				{
				case 3:
				case 4:
					tokens3.emplace_back(rando_test_code.substr(prev, idx - prev));
					prev = idx;
					state = 0;
					break;
				default:
					break;
				}
				idx++;
			}
		}
	});
	
	//a look-alike to what I originally wrote
	benchmark.run("hand_coded break to switch", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens4.clear();
		uint32_t state = 0;
		while (idx < rando_test_code.size()) {
			for (; idx < rando_test_code.size();) {
				uint8_t chr = rando_test_code[idx];
				uint8_t eqc = custom_class_table[chr];
				state = custom_table[state][eqc];
				if (state > 2) [[unlikely]] {
					break;
				}
				idx++;
			}

			switch (state)
			{
			case 3:
			case 4:
				tokens4.emplace_back(rando_test_code.substr(prev, idx - prev));
				break;
			default:
				break;
			}
			prev = idx;
			state = 0;
		}
	});

	benchmark.run("ctre::evaluate_search_string", [&]() {
		size_t idx = 0;
		tokens.clear();
		for (; idx < rando_test_code.size();) {
			auto m = ctre::starts_with<spacing_patterns>(rando_test_code.substr(idx));
			if (!m) [[unlikely]]
				break;
			tokens.emplace_back(m.to_view());
			idx += m.size();
		}
	});


	benchmark.run("ctre::evaluate_search_string overlap_strs", [&]() {
		size_t idx = 0;
		tokens.clear();
		for (; idx < rando_test_code.size();) {
			auto m = ctre::starts_with<overlap_strs>(rando_test_code.substr(idx));
			if (!m) [[unlikely]]
				break;
			tokens.emplace_back(m.to_view());
			idx += m.size();
		}
		});
	/*
	benchmark.run("ctre::search_for_string", [&]() {
		size_t idx = 0;
		tokens.clear();
		for (; idx < rando_test_code.size();) {
			auto m = ctre::search_for(rando_test_code.begin()+idx, rando_test_code.end(), ctre::regex_builder<spacing_patterns>::type{});
			tokens.emplace_back(m.to_view());
			idx += m.size();
		}
	});
	*/
	return 0;
}