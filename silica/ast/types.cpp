#include "types.h"
namespace Silica {
	namespace Types {
		using namespace std::literals;
		const Integer Int64   { true,  8, "Int64"sv };
		const Integer Int32   { true,  4, "Int32"sv };
		const Integer Int16   { true,  2, "Int16"sv };
		const Integer Int8    { true,  1, "Int8"sv };

		const Integer UInt64  { false, 8, "UInt64"sv };
		const Integer UInt32  { false, 4, "UInt32"sv };
		const Integer UInt16  { false, 2, "UInt16"sv };
		const Integer UInt8   { false, 1, "UInt8"sv };

		// used to ref a UTF-N element
		const Integer Char32  { false, 4, "Char32"sv };
		const Integer Char16  { false, 2, "Char16"sv };
		const Integer Char8   { false, 1, "Char8"sv };

		const Float   Float32 { 4, "Float32"sv };
		const Float   Float64 { 8, "Float64"sv };

		const Type    Void;

		const std::array<const Type*, 14> all = { &Int64, &Int32, &Int16, &Int8, &UInt64, &UInt32, &UInt16, &UInt8, &Char32, &Char16, &Char8, &Float32, &Float64, &Void };
	};
}
