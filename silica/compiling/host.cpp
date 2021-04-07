#include "host.h"

namespace Silica {
#if HOST == HOST_WIN
	void* fancyAlloc(size_t size, Rights rights) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		auto const page_size = system_info.dwPageSize;
		void* result = VirtualAlloc(NULL, page_size, MEM_RESERVE | MEM_COMMIT, DWORD(rights));
		if (result == nullptr) {
			throw BadFancyAlloc();
		} else {
			return result;
		}
	}

	void fancyProtect(void* ptr, size_t size, Rights rights) {
		DWORD dummy;
		if (VirtualProtect(ptr, size, DWORD(rights), &dummy) == 0) {
			throw BadFancyAlloc();
		}
		printf("protecc");
	}
#elif HOST = HOST_POSIX
	// Untested
	void* fancyAlloc(size_t size, Rights rights) {
		void* result = mmap(nullptr, size, int(rights), MAP_ANON | MAP_SHARED, -1, 0);
		if (result == MAP_FAILED) {
			throw BadFancyAlloc();
		}
		else {
			return result;
		}
	}

	void fancyProtect(void* ptr, size_t size, Rights rights) {
		if (mprotect(ptr, size, int(rights)) != 0) {
			throw BadFancyAlloc();
		}
	}

#endif
}
