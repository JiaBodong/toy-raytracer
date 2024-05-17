#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "aabb.h"
class material;

struct hit_record
{
    float t;//hit time
    vec3 p;//hit point
    vec3 normal;//normal at hit point
    material *mat_ptr;//material of the hit object
};

class hitable  {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
        virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};


#endif