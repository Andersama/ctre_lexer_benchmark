// ctre_lexer.cpp : Defines the entry point for the application.
//

#include "nanobench.h"

#include "ctre.hpp"
#include <experimental/vector>
#include <string_view>
#include <iostream>
#include <array>

using namespace std;

static constexpr auto spacing_patterns = ctll::fixed_string(
	R"RGX([\s]+|[\S]+)RGX"
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

	constexpr auto custom_class_table = []() {
		std::array<uint8_t, 256> dfa_table{};
		for (size_t i = 0; i < dfa_table.size(); i++) {
			dfa_table[i] = ctre::space_chars::match_char((uint8_t)i);
		}
		return dfa_table;
	}();

	constexpr auto custom_table = []() {
		std::array<std::array<uint8_t, 2>, 3> dfa_table{};
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
			if (state > 2) {
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
				if (state > 2) {
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
	
	//a look-alike to what I originally wrote
	benchmark.run("hand_coded switch", [&]() {
		size_t prev = 0;
		size_t idx = 0;
		tokens4.clear();
		uint32_t state = 0;
		while (idx < rando_test_code.size()) {
			for (; idx < rando_test_code.size();) {
				uint8_t chr = rando_test_code[idx];
				uint8_t eqc = custom_class_table[chr];
				state = custom_table[state][eqc];
				if (state > 2) {
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

	benchmark.run("ctre::starts_with", [&]() {
		size_t idx = 0;
		tokens.clear();
		for (; idx < rando_test_code.size();) {
			auto m = ctre::starts_with<spacing_patterns>(rando_test_code.substr(idx));
			tokens.emplace_back(m.to_view());
			idx += m.size();
		}
	});

	return 0;
}
