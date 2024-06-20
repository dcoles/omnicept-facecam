#ifndef MACROS_HPP
#define MACROS_HPP

#include <iostream>

#define LOG(X) do { std::cerr << X << "\n"; } while(false)
#define STR(X) #X
#define XSTR(X) STR(X)

#endif // MACROS_HPP
