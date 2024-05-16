#ifndef RAYH
#define RAYH

#include "vec3.h"


class ray
{
    public:
        ray() {}
        ray(const vec3& a, const vec3& b, float time=0.0) { A = a; B = b; tm=time;}//ray = a + tb
        vec3 origin() const       { return A; }
        vec3 direction() const    { return B; }
        float time() const        { return tm; };//for motion blur
        vec3 point_at_parameter(float t) const { return A + t*B; }
    private:
        vec3 A;
        vec3 B;
        float tm;
};


#endif