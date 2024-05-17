#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"


class sphere: public hitable  {
    public:
        sphere() {}
        sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat_ptr(m)  {};
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        virtual bool bounding_box(float t0, float t1, aabb& box) const;
        vec3 center;
        float radius;
        material *mat_ptr;
};

//ray-sphere intersection
bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {

    vec3 oc = r.origin() - center;//ray origin - sphere center
    
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - a*c;//discriminant of quadratic equation

    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;//update hit time
            rec.p = r.point_at_parameter(rec.t);//update hit point
            rec.normal = (rec.p - center) / radius;//update normal direction
            rec.mat_ptr = mat_ptr;//tell this point's material
            return true;
        }
        //if the first intersection is behind the camera, check the second intersection
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}

//bounding box of a sphere
bool sphere::bounding_box(float t0, float t1, aabb& box) const {
    box = aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
    return true;
}

#endif