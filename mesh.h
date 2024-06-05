#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "hitable.h"
#include "material.h"
#include "external/tiny_obj_loader.h"
#include <vector>
#include <string>
#include <iostream>


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

// mesh obj class
class mesh : public hitable {
    public:
        mesh(const std::string& filename, material *mat, float scale = 1.0, const vec3& offset = vec3(0, 0, 0)) {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), nullptr, true)) {
                std::cerr << err << std::endl;
                return;
            }
            for (size_t s = 0; s < shapes.size(); s++) {
                size_t index_offset = 0;
                for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                    int fv = shapes[s].mesh.num_face_vertices[f];
                    if (fv != 3) {
                        std::cerr << "Only triangle mesh is supported" << std::endl;
                        return;
                    }
                    std::vector<vec3> vertices;
                    for (size_t v = 0; v < fv; v++) {
                        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                        vertices.push_back(scale*vec3(attrib.vertices[3*idx.vertex_index+0],
                                                 attrib.vertices[3*idx.vertex_index+1],
                                                 attrib.vertices[3*idx.vertex_index+2]));
                    }
                    for (auto& vertex : vertices) vertex += offset;//translate the mesh
                    triangles.push_back(new triangle(vertices[0], vertices[1], vertices[2], mat));
                    index_offset += fv;
                }
            }
        }

        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override {
            hit_record temp_rec;
            bool hit_anything = false;
            float closest_so_far = t_max;
            for (const auto& triangle : triangles) {
                if (triangle->hit(r, t_min, closest_so_far, temp_rec)) {
                    hit_anything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }
            return hit_anything;
        }

        virtual bool bounding_box(float t0, float t1, aabb& box) const override {
            if (triangles.empty()) return false;
            aabb temp_box;
            bool first_box = true;
            for (const auto& triangle : triangles) {
                if (!triangle->bounding_box(t0, t1, temp_box)) return false;
                box = first_box ? temp_box : box.merge_box(box, temp_box);
                first_box = false;
            }
            return true;
        }

        std::vector<triangle*> triangles;


};





#endif
