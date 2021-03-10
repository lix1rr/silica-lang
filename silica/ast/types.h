#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <numeric>
#include <algorithm>

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
	inline const Integer Int64  {true,  8, "Int64"sv};
	inline const Integer Int32  {true,  4, "Int32"sv};
	inline const Integer Int16  {true,  2, "Int16"sv};
	inline const Integer Int8   {true,  1, "Int8"sv};

	inline const Integer UInt64 {false, 8, "UInt64"sv};
	inline const Integer UInt32 {false, 4, "UInt32"sv};
	inline const Integer UInt16 {false, 2, "UInt16"sv};
	inline const Integer UInt8  {false, 1, "UInt8"sv};

	// used to ref a UTF-N element
	inline const Integer Char32 {false, 4, "Char32"sv};
	inline const Integer Char16 {false, 2, "Char16"sv};
	inline const Integer Char8  {false, 1, "Char8"sv};

	inline const Float   Float32 { 4, "Float32"sv};
	inline const Float   Float64 { 8, "Float64"sv};

	inline const Type    Void;
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