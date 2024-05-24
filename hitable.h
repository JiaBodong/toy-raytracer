#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "aabb.h"
class material;

struct hit_record
{
    float t; // hit time
    float u, v; // texture coordinates
    vec3 p; // hit point
    vec3 normal; // normal at hit point
    material* mat_ptr; // material of the hit object

    inline void set_face_normal(const ray& r, const vec3& outward_normal)
    {
        auto front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hitable  {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
        virtual bool bounding_box(float t0, float t1, aabb& box) const = 0;
};


#endif