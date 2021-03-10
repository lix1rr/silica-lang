#pragma once
#include "include.h"
#include "prism library/prism-lib.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <numbers>


#ifndef PROJECT_DIR
#error "Project directory not defined"
#else
#define TESTS_DIR_PREFIX PROJECT_DIR "/tests/"
#endif

namespace Silica {
constexpr int startTest = 5;
constexpr int endTest = 5;


template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> vec) {
	for (size_t i = 0; i < vec.size(); i++) {
		std::cout << '[' << i << "] " << vec[i] << '\n';
	}
	return os;
}



inline bool test() {
	for (size_t i = startTest; i <= endTest; i++) {
		std::string file = std::string(TESTS_DIR_PREFIX) + "test" + std::to_string(i) + ".silica";
		std::ifstream stream {file};
		if (!stream.is_open()) {
			std::cerr << "Couldn't open file " << file;
			continue;
		}

		
		// Run test
		std::cout << "Test " << i << "\n";
		std::stringstream programOutput;
		outStream = &programOutput;
		std::stringstream log;
		auto start = std::chrono::high_resolution_clock::now();
		std::optional<double> returnVal = run(stream, file, std::cout, true);
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "\nReturn value = " << (returnVal.has_value() ? std::to_string(returnVal.value()) : "nil") 
		          << ",\nCompleted in "
		          << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start).count()
				  << "ms\n";
		stream.close();		
	}
	return false;
}

}