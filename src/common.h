#pragma once

// ----------------------------------------------------------------------------
// Parameter constants
// ----------------------------------------------------------------------------

static constexpr double Pi = 3.141592653589793;
static constexpr double InvPi = 0.3183098861837907;

// -----------------------------------------------------------------------------
// Assertion with message
// -----------------------------------------------------------------------------

#ifndef __FUNCTION_NAME__
#    if defined(_WIN32) || defined(__WIN32__)
#        define __FUNCTION_NAME__ __FUNCTION__
#    else
#        define __FUNCTION_NAME__ __func__
#    endif
#endif

#undef NDEBUG
#ifdef PY_VERSION_HEX
#    define Assertion(PREDICATE, ...)                                  \
        do {                                                           \
            if (!(PREDICATE)) {                                        \
                std::stringstream ss;                                  \
                ss << "Asssertion \""                                  \
                   << #PREDICATE << "\" failed in " << __FILE__        \
                   << " line " << __LINE__                             \
                   << " in function \"" << (__FUNCTION_NAME__) << "\"" \
                   << " : ";                                           \
                throw std::runtime_error(ss.str());                    \
            }                                                          \
        } while (false)
#elif !defined(NDEBUG)
#    define Assertion(PREDICATE, ...)                                         \
        do {                                                                  \
            if (!(PREDICATE)) {                                               \
                std::cerr << "Asssertion \""                                  \
                          << #PREDICATE << "\" failed in " << __FILE__        \
                          << " line " << __LINE__                             \
                          << " in function \"" << (__FUNCTION_NAME__) << "\"" \
                          << " : ";                                           \
                fprintf(stderr, __VA_ARGS__);                                 \
                std::cerr << std::endl;                                       \
                std::abort();                                                 \
            }                                                                 \
        } while (false)
#else  // NDEBUG
#    define Assertion(PREDICATE, ...) \
        do {                          \
        } while (false)
#endif  // NDEBUG

// -----------------------------------------------------------------------------
// Message handlers
// -----------------------------------------------------------------------------

#ifndef NDEBUG
#    define Info(...)                     \
        do {                              \
            std::cout << "[ INFO  ] ";    \
            fprintf(stdout, __VA_ARGS__); \
            std::cerr << std::endl;       \
        } while (false);
#    define Warning(...)                  \
        do {                              \
            std::cerr << "[WARNING] ";    \
            fprintf(stdout, __VA_ARGS__); \
            std::cerr << std::endl;       \
        } while (false);
#else
#    define Info(...)
#    define Warning(...)
#endif

#define Error(...)                    \
    do {                              \
        std::cerr << "[ ERROR ] ";    \
        fprintf(stderr, __VA_ARGS__); \
        std::cerr << std::endl;       \
        std::abort();                 \
    } while (false);
