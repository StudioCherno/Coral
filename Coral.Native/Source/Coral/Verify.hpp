#pragma once

#ifdef __clang__
	#define CORAL_HAS_SOURCE_LOCATION 0
#else
	#define CORAL_HAS_SOURCE_LOCATION __has_include(<source_location>)
#endif

#if CORAL_HAS_SOURCE_LOCATION
#include <source_location>

#define CORAL_SOURCE_LOCATION std::source_location location = std::source_location::current(); const char* file = location.file_name(); int line = location.line()
#else
#define CORAL_SOURCE_LOCATION const char* file = __FILE__; int line = __LINE__
#endif

#if defined(__clang__)
	#define CORAL_DEBUG_BREAK __builtin_debugtrap()
#elif defined(_MSC_VER)
	#define CORAL_DEBUG_BREAK __debugbreak()
#else
	#define CORAL_DEBUG_BREAK	
#endif

#define CORAL_VERIFY(expr) {\
						if(!(expr))\
						{\
							CORAL_SOURCE_LOCATION;\
							std::cerr << "[Coral.Native]: Assert Failed! Expression: " << #expr << " at " << file << ":" << line << "\n";\
							CORAL_DEBUG_BREAK;\
						}\
					}
