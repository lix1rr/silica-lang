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

xed_state_t& xedState() {
	// C++ static variable initialization
	struct Init {
		xed_state_t state;
		Init() {
            xed_tables_init();
			xed_state_zero(&state);
			state.stack_addr_width = XED_ADDRESS_WIDTH_64b;
			state.mmode = XED_MACHINE_MODE_LONG_64;
		}
	};
	static Init init;
	return init.state;
}

namespace Silica {
	void compilerTest() {
		xed_encoder_instruction_t x;
		//xed_inst2(&x, xedState(), XED_ICLASS_XOR, 64, xed_reg(XED_REG_RAX), xed_imm0(324, 64));
		xed_inst2(&x, xedState(), XED_ICLASS_XOR, 0,
			xed_reg(XED_REG_ECX),
			xed_reg(XED_REG_EDX));
		xed_encoder_request_t enc_req;
		xed_encoder_request_zero_set_mode(&enc_req, &x.mode);
		xed_bool_t convert_ok = xed_convert_to_encoder_request(&enc_req, &x);
		if (!convert_ok) {
			fprintf(stderr, "conversion to encode request failed\n");
			return;
		}

		xed_uint8_t itext[XED_MAX_INSTRUCTION_BYTES];
		unsigned int len = XED_MAX_INSTRUCTION_BYTES;
		unsigned int olen = 0;
		xed_error_enum_t xed_error = xed_encode(&enc_req, itext, len, &olen);
		if (xed_error != XED_ERROR_NONE) {
			fprintf(stderr, "ENCODE ERROR: %s\n",
				xed_error_enum_t2str(xed_error));
			return;
		}
		printf("Result: ");
		for (int j = 0; j < olen; j++)
			printf("%02x ", itext[j]);


		
		//Compiler compiler;
		//compiler.sections.push_back(Section(Rights::code));
		//auto& code = compiler.sections.back().data;
		//code.insert(code.end(), {
		//	0xb8,                   // move the following value to EAX:
		//	0x05, 0x00, 0x00, 0x00, // 5
		//	0xc3                    // return what's currently in EAX
		//});
		//std::cout << compiler.run(/*section*/ 0, /*offset*/ 0);
	}
}

std::ofstream allocLog;

bool hideAllocation = true;

void* operator new(size_t count) {
	if (!hideAllocation) {
		allocLog << "(new " << count << ")\n";
	};
	size_t* ptr = (size_t*) malloc(count + sizeof(size_t));
	if (ptr == nullptr) { return ptr; };
	ptr[0] = count;
	ptr++;
	return ptr;
}

void operator delete(void* ptr) {
	if (ptr == nullptr && !hideAllocation) {
		hideAllocation = true;
		allocLog << "(delete nullptr)\n";
		hideAllocation = false;
		return;
	}
	size_t* x = (size_t*)ptr;
	x--;
	size_t count = x[0];
	free(x);
	if (!hideAllocation) {
		hideAllocation = true;
		allocLog << "(delete " << count << ")\n";
		hideAllocation = false;
	}
}

int main() {
	allocLog = std::ofstream("log.txt");
	if (!allocLog) {
		std::cerr << "unable to open log";
		return EXIT_FAILURE;
	}

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
		hideAllocation = false;
		Silica::test();
		hideAllocation = true;
		std::cout << "\nend\n";
	}
	catch (std::bad_alloc&) {
		std::cout << "Out of memory\n";
	}
	catch (std::exception& e) {
		std::cout << "Exception: " << e.what();
	}
}