//! Utility macros.

#ifndef MACROS_HPP
#define MACROS_HPP

#include <iostream>

#ifdef _WIN32
#   define OS_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#else
#   define OS_OTHER
#endif

#define LOG(X) do { std::cerr << X << "\n"; } while (false)
#define LOG_ERROR(X) LOG( "ERROR: " << X)
#define STR(X) #X
#define XSTR(X) STR(X)

#endif // MACROS_HPP
