#pragma once
#include <optional>
#include <iostream>
#include <filesystem>

namespace Silica {
	constexpr std::string_view target = TARGET_OS "-" TARGET_PROCESSOR;
	std::optional<double> run(std::istream& stream, std::string name, std::ostream& outStream, bool emitIR);
};

namespace Options {
	extern bool useColour;
	extern bool useUnicode;
}

#define unreachable() std::cerr << "Reached what is supposedly unreachable code!"


#ifdef _DEBUG
	#define myAssert(expr, ...) myAssertFunc(#expr, expr, __FILE__, __LINE__, __VA_ARGS__)
#else
	#define myAssert(expr, ...)
#endif

inline void myAssertFunc(std::string exprStr, bool expr, std::filesystem::path file, int line, std::string_view msg = "") {
	if (!expr) {
		std::cerr 
			<< "Debug assertion failed!\n"
			<< msg
			<< "\nExpression '" << exprStr << "' evaluated to false."
			<< "File: " << file.filename() << ':' << line << '\n';
		// <-- Breakpoint?
		throw std::runtime_error("Debug assertion failed!");
	}
}
