#pragma once
#include <iostream>
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif
namespace Silica {
	extern std::ostream* outStream;
}


EXPORT double printDouble(double);
EXPORT double printUnicode(double);
EXPORT double getDouble();
EXPORT double printByte(double);
