#ifndef SAH_H
#define SAH_H

#include "hitable.h"
#include "hitable_list.h"
#include "random.h"
#include <algorithm>
#include <vector>
//bounding volume hierarchy, based on the hitable_list
class sah_node : public hitable {
    public:
        sah_node() {}
        sah_node(hitable **l, int n, float time0, float time1){
            //randomly choose an axis to sort the objects
            auto object_list = l;
            std::vector<aabb> boxes(n);
            std::vector<float> areas(n);
            for (int i = 0; i < n; i++) {
                if (!object_list[i]->bounding_box(time0, time1, boxes[i]))
                    std::cerr << "no bounding box in sah_node constructor\n";
                areas[i] = boxes[i].area();
            }

            int axis = int(3*drand48());
            std::sort(object_list, object_list + n, [axis, boxes](const hitable *a, const hitable *b) {
                aabb box_left, box_right;
                if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
                    std::cerr << "no bounding box in sah_node constructor\n";
                return box_left.min()[axis] < box_right.min()[axis];
            });

            //sweep and partition
            float min_cost = FLT_MAX;
            int min_index = 0;
            for (int i = 1; i < n; i++) {
                aabb box_left, box_right;
                float cost = i * boxes[i].area();
                if (!object_list[i]->bounding_box(0, 0, box_right))
                    std::cerr << "no bounding box in sah_node constructor\n";
                cost += (n - i) * box_right.area();
                if (cost < min_cost) {
                    min_cost = cost;
                    min_index = i;
                }
            }

            //recrusively construct the bounding volume hierarchy
            if (n == 1) {
                left = right = object_list[0];
            } else if (n == 2) {
                left = object_list[0];
                right = object_list[1];
            } else {
                left = new sah_node(object_list, min_index, time0, time1);
                right = new sah_node(object_list + min_index, n - min_index, time0, time1);
            }

            aabb box_left, box_right;
            if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
                std::cerr << "no bounding box in sah_node constructor\n";
            box = box.merge_box(box_left, box_right);
            

        }
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        virtual bool bounding_box(float t0, float t1, aabb& box) const;
        hitable *left;//not tratitional left and right, can be any two other nodes
        hitable *right;
        aabb box;

};

bool sah_node::bounding_box(float t0, float t1, aabb& b) const {
    b = box;
    return true;
}

bool sah_node::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
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