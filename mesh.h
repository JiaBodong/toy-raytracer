#ifndef MESH_H
#define MESH_H

#include "hitable.h"
#include "material.h"
#include <vector>

// Triangle class for mesh
class triangle : public hitable {
    public:
        triangle() {}
        triangle(const vec3& v0, const vec3& v1, const vec3& v2, material *mat)
            : v0(v0), v1(v1), v2(v2), mp(mat) {}
        //ray-triangle intersection
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;
        virtual bool bounding_box(float t0, float t1, aabb& box) const override {//bounding box of a triangle
            vec3 min(fmin(v0.x(), fmin(v1.x(), v2.x())),
                     fmin(v0.y(), fmin(v1.y(), v2.y())),
                     fmin(v0.z(), fmin(v1.z(), v2.z())));
            vec3 max(fmax(v0.x(), fmax(v1.x(), v2.x())),
                     fmax(v0.y(), fmax(v1.y(), v2.y())),
                     fmax(v0.z(), fmax(v1.z(), v2.z())));
            box = aabb(min, max);
            return true;
        }

        material *mp;
        vec3 v0, v1, v2;//vertices of the triangle

    private:
        bool intersect_triangle(const ray& r, float& t, float& u, float& v) const;
};
//ray-triangle intersection, Moller-Trumbore algorithm
bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    float t, u, v;
    if (intersect_triangle(r, t, u, v) && t < t_max && t > t_min) {
        rec.t = t;
        rec.p = r.point_at_parameter(t);
        vec3 normal = unit_vector(cross(v1 - v0, v2 - v0));
        rec.set_face_normal(r, normal);
        rec.u = u;
        rec.v = v;
        rec.mat_ptr = mp;
        return true;
    }
    return false;
}

bool triangle::intersect_triangle(const ray& r, float& t, float& u, float& v) const {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(r.direction(), edge2);
    float a = dot(edge1, h);
    if (fabs(a) < 0.00001) return false;//ray and triangle are parallel
    float f = 1.0 / a;
    vec3 s = r.origin() - v0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;//ray misses the triangle
    vec3 q = cross(s, edge1);
    v = f * dot(r.direction(), q);
    if (v < 0.0 || u + v > 1.0) return false;//ray misses the triangle
    t = f * dot(edge2, q);
    return t > 0.00001;
}

// Mesh class, a collection of triangles
class mesh : public hitable {
    public:
        mesh() {}
        mesh(const std::vector<triangle*>& tris) : triangles(tris) {}
        //ray-mesh intersection
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;
        virtual bool bounding_box(float t0, float t1, aabb& box) const override {
            if (triangles.empty()) return false;
            aabb temp_box;
            bool first_box = true;
            //loop through all triangles to get the bounding box of the mesh
            for (const auto& tri : triangles) {
                if (!tri->bounding_box(t0, t1, temp_box)) return false;
                box = first_box ? temp_box : box.merge_box(box, temp_box);
                first_box = false;
            }
            return true;
        }
        std::vector<triangle*> triangles;
};

bool mesh::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;
    //loop through all triangles to find the closest hit
    for (const auto& tri : triangles) {
        if (tri->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}

#endif
