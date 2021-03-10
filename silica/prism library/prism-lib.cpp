#include "prism-lib.h"
#include <cmath>
#include <iostream>
#include <string>

namespace Silica {
	std::ostream* outStream = &std::cout;
}

using namespace Silica;


double printByte(double val) {
	if (trunc(val) != val) {
		return -1;
	}
	if (val < 0 || val > 0xFF) {
		return -2;
	}
	outStream->put((char) val);
	return 0;
}

double printDouble(double val) {
	*outStream << val;
	return 0.0;
}

double printUnicode(double floatVal) {
	if ((trunc(floatVal) != floatVal) || floatVal > 0x10FFFF) {
		//error, replacement char
		*outStream << (char)0xEF
			 << (char)0xBF
			 << (char)0xBD;
		return 0;
	}
	uint32_t utf = floatVal;

	if (utf <= 0x7F) {
		// Plain ASCII
		*outStream << (char) utf;
		return 1;
	}
	else if (utf <= 0x07FF) {
		// 2-byte unicode
		*outStream << (char)(((utf >>  6) & 0x1F) | 0xC0)
			 << (char)(((utf >>  0) & 0x3F) | 0x80);
		return 2;
	}
	else if (utf <= 0xFFFF) {
		// 3-byte unicode
		*outStream << (char)(((utf >> 12) & 0x0F) | 0xE0)
			 << (char)(((utf >>  6) & 0x3F) | 0x80)
			 << (char)(((utf >>  0) & 0x3F) | 0x80);
		return 3;
	}
	else if (utf <= 0x10FFFF) {
		// 4-byte unicode
		*outStream << (char)(((utf >> 18) & 0x07) | 0xF0)
			 << (char)(((utf >> 12) & 0x3F) | 0x80)
			 << (char)(((utf >>  6) & 0x3F) | 0x80)
			 << (char)(((utf >>  0) & 0x3F) | 0x80);
		return 4;
	}
	else {
		// error - use replacement character
		*outStream << (char)0xEF
			 << (char)0xBF
			 << (char)0xBD;
		return -1;
	}

}

double getDouble() {
	return 213.4;
}