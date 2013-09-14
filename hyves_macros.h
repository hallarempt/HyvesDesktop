#ifndef HYVES_MACROS
#define HYVES_MACROS

/* #undef __HYVES_HAVE_GCC_VISIBILITY */

#ifdef __HYVES_HAVE_GCC_VISIBILITY
#	define HYVES_NO_EXPORT __attribute__ ((visibility("hidden")))
#	define HYVES_EXPORT __attribute__ ((visibility("default")))
#	define HYVES_IMPORT __attribute__ ((visibility("default")))
#elif defined(_WIN32) || defined(_WIN64)
#	define HYVES_NO_EXPORT
#	define HYVES_EXPORT __declspec(dllexport)
#	define HYVES_IMPORT __declspec(dllimport)
#else
#	define HYVES_NO_EXPORT
#	define HYVES_EXPORT
#	define HYVES_IMPORT
#endif

#endif

