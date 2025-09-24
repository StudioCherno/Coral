#pragma once

#define CORAL_SOURCE_LOCATION const char* file = __FILE__; int line = __LINE__

#if defined(__GNUC__)
	#define CORAL_DEBUG_BREAK __builtin_trap()
#elif defined(_MSC_VER)
	#define CORAL_DEBUG_BREAK __debugbreak()
#else
	#define CORAL_DEBUG_BREAK	
#endif

#define CORAL_VERIFY(expr) do {\
						if(!(expr))\
						{\
							CORAL_SOURCE_LOCATION;\
							std::cerr << "[Coral.Native]: Assert Failed! Expression: " << #expr << " at " << file << ":" << line << "\n";\
							CORAL_DEBUG_BREAK;\
						}\
					} while(0)
