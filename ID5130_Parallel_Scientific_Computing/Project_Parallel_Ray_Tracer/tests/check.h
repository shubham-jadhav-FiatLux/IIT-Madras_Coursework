#pragma once
#include <cmath>
#include <iostream>

#define CHECK(cond)                                                           \
    do {                                                                      \
        if (!(cond)) {                                                        \
            std::cerr << __FILE__ << ":" << __LINE__                          \
                      << " CHECK(" #cond ") failed\n";                        \
            return 1;                                                         \
        }                                                                     \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                 \
    do {                                                                      \
        const double _va = (a), _vb = (b);                                    \
        if (!(std::abs(_va - _vb) <= (eps))) {                                \
            std::cerr << __FILE__ << ":" << __LINE__                          \
                      << " CHECK_NEAR(" #a ", " #b ") failed: "               \
                      << _va << " vs " << _vb << " (eps=" << (eps) << ")\n";  \
            return 1;                                                         \
        }                                                                     \
    } while (0)
