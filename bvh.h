#ifndef BVH_H
#define BVH_H

#include "hitable.h"
#include "hitable_list.h"
#include <algorithm>
#include <vector>
//bounding volume hierarchy, based on the hitable_list
class bvh_node : public hitable {
    public:
        bvh_node() {}
        bvh_node(hitable **l, int n, float time0, float time1){
            auto object_list = l;
            int axis = int(3*drand48());
            if (axis == 0)
                std::sort(object_list, object_list + n, box_x_compare);
            else if (axis == 1)
                std::sort(object_list, object_list + n, box_y_compare);
            else
                std::sort(object_list, object_list + n, box_z_compare);
            if (n == 1) {
                left = right = object_list[0];
            } else if (n == 2) {
                left = object_list[0];
                right = object_list[1];
            } else {
                left = new bvh_node(object_list, n/2, time0, time1);
                right = new bvh_node(object_list + n/2, n - n/2, time0, time1);
            }
            aabb box_left, box_right;
            if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
                std::cerr << "no bounding box in bvh_node constructor\n";
            box = box.merge_box(box_left, box_right);
            

        }
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        virtual bool bounding_box(float t0, float t1, aabb& box) const;
        hitable *left;//not tratitional left and right, can be any two other nodes
        hitable *right;
        aabb box;

        static bool box_x_compare(const hitable *a, const hitable *b) {
            aabb box_left, box_right;
            if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
                std::cerr << "no bounding box in bvh_node constructor\n";
            return box_left.min().x() < box_right.min().x();
        }
        static bool box_y_compare(const hitable *a, const hitable *b) {
            aabb box_left, box_right;
            if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
                std::cerr << "no bounding box in bvh_node constructor\n";
            return box_left.min().y() < box_right.min().y();
        }
        static bool box_z_compare(const hitable *a, const hitable *b) {
            aabb box_left, box_right;
            if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
                std::cerr << "no bounding box in bvh_node constructor\n";
            return box_left.min().z() < box_right.min().z();
        }

};

bool bvh_node::bounding_box(float t0, float t1, aabb& b) const {
    b = box;
    return true;
}

bool bvh_node::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    if (box.hit(r, t_min, t_max)) {
        hit_record left_rec, right_rec;

        //recursively check the left and right nodes
        bool hit_left = left->hit(r, t_min, t_max, left_rec);
        bool hit_right = right->hit(r, t_min, t_max, right_rec);
        if (hit_left && hit_right) {
            if (left_rec.t < right_rec.t)
                rec = left_rec;
            else
                rec = right_rec;
            return true;
        } else if (hit_left) {
            rec = left_rec;
            return true;
        } else if (hit_right) {
            rec = right_rec;
            return true;
        } else
            return false;
    } else
        return false;
}

#endif