#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <array>

namespace Silica {
struct Type {
	/*
		Scalar type: llvm type
		Tuple type: members (types + names)
		Distinct type: (name + underlying type)
	*/
	virtual ~Type() {};
	// In bytes
	uint64_t size;
	uint64_t align;
	bool isFloating;
	bool isVoid;
	std::string_view name;
	constexpr Type(uint64_t size, uint64_t align, bool isFloating, std::string_view name) :
		size(size), align(align), isFloating(isFloating), isVoid(false), name(name) {
	};
	constexpr Type() : size(0), align(1), isFloating(false), isVoid(true) {};
};

inline bool operator==(const Type& a, const Type& b) {
	// 'Void' types are not distinct
	if (a.isVoid && b.isVoid) {
		return true;
	} else {
		return &a == &b;
	}
}

struct Integer: public Type {
	bool isSigned;

	constexpr Integer(bool isSigned, uint64_t size, std::string_view name) :
		Type(size, size, false, name), isSigned(isSigned) {
	};

	constexpr Integer(bool isSigned, uint64_t size, uint64_t align, std::string_view name) :
		Type(size, align, false, name), isSigned(isSigned) {
	};
};

struct Float: public Type {
	constexpr Float(uint64_t size, std::string_view name) :
		Type(size, size, true, name) {
	};
	constexpr Float(uint64_t size, uint64_t align, std::string_view name) :
		Type(size, align, true, name) {
	};
};

namespace Types {
	using namespace std::literals;
	extern const Integer Int64;
	extern const Integer Int32;
	extern const Integer Int16;
	extern const Integer Int8;

	extern const Integer UInt64;
	extern const Integer UInt32;
	extern const Integer UInt16;
	extern const Integer UInt8;

			// used to ref a UTF-N element
	extern const Integer Char32;
	extern const Integer Char16;
	extern const Integer Char8;

	extern const Float   Float32;
	extern const Float   Float64;

	extern const Type    Void;

	extern const std::array<const Type*, 14> all;
};

struct Tuple : public Type {
	std::vector<std::pair<std::string, Type>> members;
	Tuple(std::vector<std::pair<std::string, Type>> theMembers): members(std::move(theMembers)) {
		if (members.size() == 0) {
			// Empty tuple is Void
			size = 0;
			align = 1;
			isFloating = false;
			isVoid = true;
		} else {
			std::sort(members.begin(), members.end(), [](auto& pair1, auto& pair2) {
				// descending order
				return  pair1.second.size > pair2.second.size;
			});
			align = members[0].second.align;
			size = std::accumulate(members.begin(), members.end(), uint64_t(0), [](uint64_t x, auto pair) {
				return pair.second.size + x;
			});
		}

	}
};
} // end namespace Silica