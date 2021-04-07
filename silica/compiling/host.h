#pragma once
#include <stdexcept>

#define HOST_WIN   1 // x86_64 Windows
#define HOST_POSIX 2 // x86_64 Posix
#define HOST_OTHER 3 // Not any of the above

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_AMD64) || defined (_M_X64)
	#ifdef _WIN64
		// An x86_64 windows host
		#define HOST HOST_WIN
		#include "Windows.h"
	#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
		#include <unistd.h>
		#if defined(_POSIX_VERSION)
		// An x86_64 posix host
			#define HOST HOST_POSIX
		#endif
	#endif
#endif

#ifndef HOST
#define HOST HOST_OTHER
#endif

namespace Silica {
#if HOST == HOST_WIN
	enum class Rights {
		rodata = PAGE_READONLY,
		rwdata = PAGE_READWRITE,
		code   = PAGE_EXECUTE
	};
#elif HOST == HOST_POSIX
	enum class Rights {
		rodata = PROT_READ,
		rwdata = PROT_READ | PROT_WRITE,
		code   = PROT_EXEC
	};
#else
	enum class Rights {
		rodata,
		rwdata,
		code
	};
#endif

#if HOST != HOST_OTHER
	struct BadFancyAlloc : public std::bad_alloc {};
	// Allocates a region of memory 'size' bytes long with the specified rights
	void* fancyAlloc(size_t size, Rights rights);

	// Changes the right's of a memory region pointed by the result of 'fancyAlloc'
	void fancyProtect(void* ptr, size_t size, Rights rights);
#endif

}