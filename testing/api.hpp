#ifndef TESTING_API_HPP
#define TESTING_API_HPP

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define PROPTEST_HELPER_DLL_IMPORT __declspec(dllimport)
  #define PROPTEST_HELPER_DLL_EXPORT __declspec(dllexport)
  #define PROPTEST_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define PROPTEST_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define PROPTEST_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define PROPTEST_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define PROPTEST_HELPER_DLL_IMPORT
    #define PROPTEST_HELPER_DLL_EXPORT
    #define PROPTEST_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define PROPTEST_API and PROPTEST_LOCAL.
// PROPTEST_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// PROPTEST_LOCAL is used for non-api symbols.

#ifdef PROPTEST_DLL // defined if PROPTEST is compiled as a DLL
  #ifdef PROPTEST_DLL_EXPORTS // defined if we are building the PROPTEST DLL (instead of using it)
    #define PROPTEST_API PROPTEST_HELPER_DLL_EXPORT
  #else
    #define PROPTEST_API PROPTEST_HELPER_DLL_IMPORT
  #endif // PROPTEST_DLL_EXPORTS
  #define PROPTEST_LOCAL PROPTEST_HELPER_DLL_LOCAL
#else // PROPTEST_DLL is not defined: this means PROPTEST is a static lib.
  #define PROPTEST_API
  #define PROPTEST_LOCAL
#endif // PROPTEST_DLL


//#if defined(PROPTEST_APIS) && !defined(PROPTEST_IMPORTS)
//#define PROPTEST_API SYMBOL_VISIBILITY_EXPORT
//#else
//#define PROPTEST_API SYMBOL_VISIBILITY_IMPORT
//#endif

#endif
