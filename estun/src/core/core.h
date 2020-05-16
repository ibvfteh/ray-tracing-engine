#pragma once

#include "core/log.h"
#include <memory>
#include <cstring>
#include <cmath>

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define ES_PLATFORM_WINDOWS
        
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define ES_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define ES_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
 * since android is based on the linux kernel
 * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define ES_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define ES_PLATFORM_LINUX
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection

// Dynamic Link Library
#if defined ES_PLATFORM_WINDOWS
	#if defined ES_DYNAMIC_LINK
		#if defined ES_BUILD_DLL
			#define HAZEL_API __declspec(dllexport)
		#else
			#define HAZEL_API __declspec(dllimport)
		#endif
	#else
		#define HAZEL_API
	#endif

#elif defined ES_PLATFORM_LINUX
	#if defined ES_DYNAMIC_LINK
		#if defined ES_BUILD_DLL
			#define HAZEL_API __attribute__((visibility("default")))
		#else
			#define HAZEL_API
		#endif
	#else
		#define HAZEL_API
	#endif

#else
    #error "Unknown platform!"
#endif

#if 1 //defined ES_DEBUG
	// debugbreak
	#if defined ES_PLATFORM_WINDOWS
		#define ES_DEBUGBREAK() __debugbreak()
	#elif defined ES_PLATFORM_LINUX
		#include <signal.h>
		#define ES_DEBUGBREAK() raise(SIGTRAP)
	#endif // End of debugbreak

	#define ES_ENABLE_ASSERTS
#endif

#if 1 //ES_ENABLE_ASSERTS
	#define ES_ASSERT(...) {  ES_ERROR("Assertion Failed: {0}", __VA_ARGS__); ES_DEBUGBREAK();  }
	#define ES_CORE_ASSERT( ...) { ES_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); ES_DEBUGBREAK();  }
#else
	#define ES_ASSERT( ...)
	#define ES_CORE_ASSERT( ...)
#endif

#define BIT(x) (1 << x)

#define ES_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

