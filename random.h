#ifndef RANDOMH
#define RANDOMH

#include <cstdlib>
#include"vec3.h"

inline double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline uint32_t morton_code(vec3& p){
    // Morton code is a way to split 3D space into 1D space
    auto splitBy3 = [](unsigned int a) {
        a = (a | (a << 16)) & 0x030000FF;
        a = (a | (a <<  8)) & 0x0300F00F;
        a = (a | (a <<  4)) & 0x030C30C3;
        a = (a | (a <<  2)) & 0x09249249;
        return a;
    };
    unsigned int x = splitBy3(p.e[0]);
    unsigned int y = splitBy3(p.e[1]);
    unsigned int z = splitBy3(p.e[2]);
    return x | (y << 1) | (z << 2);
}
#endif