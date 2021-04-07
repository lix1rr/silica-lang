#pragma once
#include "include.h"
#include "host.h"
extern "C" {
	#include "xed/xed-interface.h"
}
#include <stdint.h>
#include <variant>
#include <vector>
#include <bitset>

using namespace std::literals;

namespace Silica {
enum class GprType {
	
};
struct Gpr {
	enum class Reg {
		a = 0, c, d, b, a_sp, c_bp, d_si, b_di, r8, r9, r10, r11, r12, r13, r14, r15
	} value;
	enum class Type {
		gp8, gp16, gp32, gp64
	} type;
	bool useLatter = false;
	// When type is gp8 and value is between a_sp and b_di, use the lower part of the right side of the underscore
};

// S = struct
// gpr = general purpose register
// N = num
// R = reg
// p = +
// t = *
// at... = [...]

// Absolute (not mutable)
// N          // Absolute                               // mov lval, num
//
// Register access (mutable)
// R          // Register                               // mov lval, rval
//
// Memory access (mutable)
// AtN        //  Displacement                          //  mov [num], rval
// AtR        //  Base                                  //  mov [gpr], rval
// AtRpR      //  Base + Index                          //  mov [gpr + gpr], rval
// AtRpN      //  Base + Displacement                   //  mov [gpr + num], rval
// AtRpRpN    //  Base + Index + Displacement           //  mov [gpr + gpr + num], rval
// AtRpNtR    //  Base + (Index * Scale)                //  mov [gpr + num*gpr], rval
// AtRtNpN    //  (Index * Scale) + Displacement        //  mov [gpr*num + num], rval
// AtRpNtRpN  //  Base + (Index * Scale) + Displacement //  mov [gpr + num*gpr + num], rval
//source:https://blog.yossarian.net/2020/06/13/How-x86_64-addresses-memory

struct Location {
	size_t size;
	struct Sibda {
		int32_t displacement;
		Gpr index;
		Gpr base;
		unsigned scale : 2;
	};
	struct RipRelative {
		int32_t displacement;
	};
	std::variant<Sibda, RipRelative> data;
	// The memory location at base + index*(2^scale) + displacement, scale is 2 bits (0..3)

	Location(unsigned scale, Gpr index, Gpr base, int32_t displacement):
		data(Sibda({displacement, index, base, scale})) {}

	Location(int32_t displacement):
		data(RipRelative({displacement})) {}
};


// REX:
// 0   1   0   0 | W | R | X | B |

enum class Prefix1: uint8_t {
	nothing = 0x00,
	lock = 0xF0,
};
enum class Prefix2: uint8_t {
	nothing = 0x00,
	branchNotTaken = 0x2E,
	branchTaken = 0x3E
};


struct Compiler;

struct Section {
	Rights rights;
	std::vector<uint8_t> data;
	size_t offset; // size_t sized placeholder for Compiler::run
	Section(Rights rights) : rights(rights) {};
};

struct Link {
	size_t pointerSection;
	size_t pointerOffset;

	size_t pointeeSection;
	size_t pointeeOffset;
};

// *((uint64_t*) &vec[index]) becomes the rip relative address to section targetSection + offset

struct BadVirtualAlloc: std::bad_alloc {};

struct Compiler {
	xed_state_t state;
	xed_encoder_instruction_t encInstruction;
	xed_encoder_request_t encRequest;
	friend Section;

	int idx = 0;
	// TODO: use deque??
	std::vector<Section> sections;
	std::vector<Link> links;

	size_t mainSection;
	size_t mainOffset;

	static Compiler compilerX64() {
		Compiler comp;
		xed_state_zero(&comp.state);
		comp.state.stack_addr_width = XED_ADDRESS_WIDTH_64b;
		comp.state.mmode = XED_MACHINE_MODE_LONG_64;
		return comp;
	}

	struct XEDError: std::runtime_error {};

	template<typename Fn, typename...OperandTypes>
	void addInstruction(Section& section, xed_uint_t operandWidth, xed_iclass_enum_t iclass, OperandTypes...operands) {
		
		if constexpr(sizeof...(operands) == 0) {
			xed_inst0(&encInstruction, state, operandWidth);
			return;
		}

		// type checking
		std::array<xed_operand_t, sizeof(OperandTypes...)> arr = { operands... };
		#define invokeXedInst(n) \
			if constexpr(sizeof...(operands) == n) {\
				xed_inst##n(&encInstruction, state, operandWidth, operands...);\
				return;\
			}

		invokeXedInst(1)
		else invokeXedInst(2)
		else invokeXedInst(3)
		else invokeXedInst(4)
		else invokeXedInst(5)
		else static_assert(false, "Requires 0..5 operands");

		if (!xed_convert_to_encoder_request(&encRequest, &encInstruction)) {
			throw XEDError{"Couldn't convert a xed instruction into an encoder request"};
		}

		size_t oldSize = sections.data.size();
		section.data.resize(oldSize + XED_MAX_INSTRUCTION_BYTES);
		size_t ptr = &section.data[oldSize];
		unsigned int olen = 0;
		xed_error_enum_t xed_error = xed_encode(&encRequest, ptr, XED_MAX_INSTRUCTION_BYTES, &olen);
		section.data.resize(oldSize + olen);

		if (xed_error != XED_ERROR_NONE) {
			throw XEDError{"Couldnt encode instruction, xed error: "sv + xed_error_enum_t2str(xed_error)};
			return;
		}
	}

	int run(size_t mainSection, size_t mainOffset) {
		std::unordered_map<Rights, std::pair<size_t, void*>> map;
		// Calculate offset and size of the code for each right
		for (Section& i : sections) {
			size_t& size = map[i.rights].first;
			i.offset = size;
			size += i.data.size();
		}
		// Allocate with read write permissions
		for (auto& pair : map) {
			if (pair.first != Rights::rwdata) {
				pair.second.second = fancyAlloc(pair.second.first, Rights::rwdata);
			}
			else {
				pair.second.second = malloc(pair.second.first);
			}
		}

		// Linking
		for (Link i : links) {
			uintptr_t* pointer = (uintptr_t*) &sections[i.pointerSection].data[i.pointerOffset];
			*pointer = uintptr_t(map[sections[i.pointeeSection].rights].second) + sections[i.pointeeSection].offset + i.pointeeOffset;
		}

		// Copy
		for (Section& i : sections) {
			std::copy(i.data.begin(), i.data.end(), (uint8_t*)map[i.rights].second + i.offset);
		}

		// Make the permission's what they are supposed to be
		for (auto& pair : map) {
			if (pair.first != Rights::rwdata) {
				fancyProtect(pair.second.second, pair.second.first, pair.first);
			}
		}

		// Run it
		auto fptr = reinterpret_cast<int(*)()>(uintptr_t(map[sections[mainSection].rights].second) + sections[mainSection].offset + mainOffset);
		int result = fptr();

		// Free

		return result;
	}
private:
	Compiler() {};
};

}
