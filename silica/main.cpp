#include "ast/ast.h"
#include "parsing/Parser.h"
#include "compiling/compiler.h"
#include "include.h"
#include <sstream>
#include <optional>
#include "tests/test.h"
extern "C" {
	#include "xed/xed-interface.h"
}

namespace Silica {
	std::optional<double> run(std::istream& stream, std::string name, std::ostream& outStream) {
		Parser parser(stream, "epic JIT", name);
		if (parser.errorCount > 0) {
			outStream << "Failed with " << parser.errorCount << " errors.\n";
			parser.printErrors(outStream);
			outStream << "AST:\n";
			parser.ast.print(outStream);
			return std::nullopt;
		}
		outStream << "Parsing success! Printing AST\n";
		parser.ast.print(outStream);
		return std::nullopt;
	}
}


namespace Options {
	bool useColour = false;
	bool useUnicode = false;
}

extern "C" void signalHandler(int signalNumber) {
	std::clog << "Failed! Recieved signal ";
	switch (signalNumber) {
	case SIGABRT:
		std::clog << "SIGABRT\n";
		return;
	case SIGFPE:
		std::clog << "SIGFPE\n";
		return;
	case SIGILL:
		std::clog << "SIGILL\n";
		return;
	case SIGINT:
		std::clog << "SIGINT\n";
		return;
	case SIGSEGV:
		std::clog << "SIGSEGV\n";
		return;
	case SIGTERM:
		std::clog << "SIGTERM\n";
		return;
	default:
		std::clog << "unknown (" << signalNumber << ")\n";
		return;
	}
}


int main() {
	signal(SIGABRT, signalHandler);
	signal(SIGFPE, signalHandler);
	signal(SIGILL, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGTERM, signalHandler);
	try {
		//Todo:fix
		//Silica::test();
		std::cout << "begin\n";
		Silica::test();
		std::cout << "\nend\n";
	}
	catch (std::bad_alloc&) {
		std::cout << "Out of memory\n";
	}
	catch (std::exception& e) {
		std::cout << "Exception: " << e.what();
	}
}