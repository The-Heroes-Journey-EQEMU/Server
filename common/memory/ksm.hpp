#ifndef EQEMU_KSM_HPP
#define EQEMU_KSM_HPP

#include <iostream>
#include <vector>
#include <cstring>
#ifdef _WIN32
#include <malloc.h>  // For _aligned_malloc, _aligned_free
#include <windows.h>
#else
#include <sys/mman.h>  // For madvise
#include <unistd.h>    // For sysconf, sbrk
#endif


// Page-aligned allocator for std::vector
template <typename T>
class PageAlignedAllocator {
public:
	using value_type = T;

	PageAlignedAllocator() noexcept = default;
	template <typename U>
	PageAlignedAllocator(const PageAlignedAllocator<U>&) noexcept {}

	T* allocate(std::size_t n) {
		void* ptr = nullptr;
		size_t size = n * sizeof(T);

#ifdef _WIN32
		// Simply allocate memory without alignment
        ptr = malloc(size);
        if (!ptr) throw std::bad_alloc();
#else
		size_t alignment = getPageSize(); // Get the system's page size
		if (posix_memalign(&ptr, alignment, size) != 0) {
			throw std::bad_alloc();
		}
#endif
		return static_cast<T*>(ptr);
	}

	void deallocate(T* p, std::size_t) noexcept {
#ifdef _WIN32
		_aligned_free(p);
#else
		free(p);
#endif
	}

private:
	size_t getPageSize() const
	{
#ifdef _WIN32
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize; // Page size in bytes
#else
		return static_cast<size_t>(sysconf(_SC_PAGESIZE)); // POSIX page size
#endif
	};
};

template <typename T, typename U>
bool operator==(const PageAlignedAllocator<T>&, const PageAlignedAllocator<U>&) noexcept {
	return true;
}

template <typename T, typename U>
bool operator!=(const PageAlignedAllocator<T>&, const PageAlignedAllocator<U>&) noexcept {
	return false;
}

// Kernel Samepage Merging (KSM) functionality
namespace KSM {

#ifdef _WIN32
	// Windows-specific placeholder functions (no-op)
    inline void CheckPageAlignment(void* ptr) {
        std::cout << "Page alignment check not supported on Windows: " << ptr << "\n";
    }

    inline void* AllocatePageAligned(size_t size) {
        size_t alignment = 4096; // Default page size for Windows
        void* aligned_ptr = _aligned_malloc(size, alignment);
        if (!aligned_ptr) {
            std::cerr << "Failed to allocate page-aligned memory on Windows." << std::endl;
        }
        std::memset(aligned_ptr, 0, size);
        return aligned_ptr;
    }

    inline void MarkMemoryForKSM(void* start, size_t size) {
    }

    inline void AlignHeapToPageBoundary() {
    }

    inline void* MarkHeapStart() {
        return nullptr;
    }

    inline size_t MeasureHeapUsage(void* start) {
        return 0;
    }
#else
	// Linux-specific functionality
	inline void CheckPageAlignment(void* ptr) {
		size_t page_size = sysconf(_SC_PAGESIZE);
		if (reinterpret_cast<uintptr_t>(ptr) % page_size == 0) {
			std::cout << "Memory is page-aligned: " << ptr << "\n";
		} else {
			std::cout << "Memory is NOT page-aligned: " << ptr << "\n";
		}
	}

	inline void* AllocatePageAligned(size_t size) {
		size_t page_size = sysconf(_SC_PAGESIZE);
		void* aligned_ptr = nullptr;
		if (posix_memalign(&aligned_ptr, page_size, size) != 0) {
			std::cerr << "Failed to allocate page-aligned memory on Linux." << std::endl;
		}
		std::memset(aligned_ptr, 0, size);
		return aligned_ptr;
	}

	inline void MarkMemoryForKSM(void* start, size_t size) {
		if (madvise(start, size, MADV_MERGEABLE) == 0) {
			std::cout << "Marked memory for KSM | start: " << start << ", size: " << size << " bytes\n";
		} else {
			perror("madvise failed");
		}
	}

	inline void AlignHeapToPageBoundary() {
		size_t page_size = sysconf(_SC_PAGESIZE);
		if (page_size == 0) {
			std::cerr << "Error: Failed to retrieve page size." << std::endl;
			return;
		}

		void* current_break = sbrk(0);
		if (current_break == (void*)-1) {
			std::cerr << "Error: Failed to retrieve the current program break." << std::endl;
			return;
		}

		uintptr_t current_address = reinterpret_cast<uintptr_t>(current_break);
		size_t misalignment = current_address % page_size;

		if (misalignment != 0) {
			size_t adjustment = page_size - misalignment;
			if (sbrk(adjustment) == (void*)-1) {
				std::cerr << "Error: Failed to align heap to page boundary." << std::endl;
				return;
			}
		}

		std::cout << "Heap aligned to next page boundary. Current break: " << sbrk(0) << std::endl;
	}

	inline void* MarkHeapStart() {
		void* current_pos = sbrk(0);
		AlignHeapToPageBoundary();
		return current_pos;
	}

	inline size_t MeasureHeapUsage(void* start) {
		void* current_break = sbrk(0);
		return static_cast<char*>(current_break) - static_cast<char*>(start);
	}
#endif


	inline size_t getPageSize()
	{
#ifdef _WIN32
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize; // Page size in bytes
#else
		return static_cast<size_t>(sysconf(_SC_PAGESIZE)); // POSIX page size
#endif
	};

	template <typename T>
	inline void PageAlignVectorAligned(std::vector<T, PageAlignedAllocator<T>>& vec) {
		if (vec.empty()) {
			std::cerr << "Vector is empty, no need to align.\n";
			return;
		}

		size_t page_size = getPageSize();
		void* start = vec.data();
		size_t size = vec.size() * sizeof(T);

		// Check if the memory is page-aligned
		if (reinterpret_cast<std::uintptr_t>(start) % page_size != 0) {
			// Allocate a new aligned vector
			std::vector<T, PageAlignedAllocator<T>> aligned_vec(vec.get_allocator());
			aligned_vec.reserve(vec.capacity()); // Match capacity to avoid reallocation during copy

			// Copy elements from the original vector
			aligned_vec.insert(aligned_vec.end(), vec.begin(), vec.end());

			// Swap the aligned vector with the original vector
			vec.swap(aligned_vec);

			// Clear the temporary aligned vector to free its memory
			aligned_vec.clear();

			// Verify the new alignment
			start = vec.data();
			if (reinterpret_cast<std::uintptr_t>(start) % page_size != 0) {
				throw std::runtime_error("Failed to align vector memory to page boundaries.");
			}

			std::cout << "Vector reallocated to ensure page alignment.\n";
		} else {
			std::cout << "Vector is already page-aligned.\n";
		}

#ifndef _WIN32
		// Mark memory for KSM (only on non-Windows systems)
		MarkMemoryForKSM(start, size);
#endif
	}

}

#endif // EQEMU_KSM_HPP
