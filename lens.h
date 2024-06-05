#ifndef LENS_H
#define LENS_H

#include "ray.h"
#include "vec3.h"

class Lens {
    public:
        Lens(vec3 pos, float radius, float refr_idx, float reflectance): position(pos), radius(radius), refractive_index(refr_idx), reflectance(reflectance) {
            
        }

        bool hit(const ray& r, float t_min, float t_max) const {
            vec3 oc = r.origin() - position;
            float a = dot(r.direction(), r.direction());
            float b = dot(oc, r.direction());
            float c = dot(oc, oc) - radius*radius;
            float discriminant = b*b - a*c;
            if (discriminant > 0) {
                float temp = (-b - sqrt(discriminant))/a;
                if (temp < t_max && temp > t_min) {
                    return true;
                }
                temp = (-b + sqrt(discriminant))/a;
                if (temp < t_max && temp > t_min) {
                    return true;
                }
            }
            return false;
        }
        bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) const{
            vec3 uv = unit_vector(v);
            float dt = dot(uv, n);
            float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
            if (discriminant > 0) {
                refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
                return true;
            }
            else
                return false;
        }

        vec3 reflect(const vec3& v, const vec3& n) const {
            return v - 2*dot(v,n)*n;
        }

        vec3 position;
        float radius;
        float refractive_index;
        float reflectance;
};

#endif

