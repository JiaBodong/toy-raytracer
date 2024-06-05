#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
#include "vec3.h"
#include "random.h"
#include <cmath>
#include <vector>

vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0 * vec3(random_double(), random_double(), 0) - vec3(1, 1, 0);
    } while (dot(p, p) >= 1.0);
    return p;
}

class camera {
public:
    camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focus_dist, float t0, float t1, int num_lens) {
        // vfov is top to bottom in degrees
        lens_radius = aperture / 2; // aperture is the diameter of the lens
        float theta = vfov * M_PI / 180; // convert to radians
        float half_height = tan(theta / 2);
        float half_width = aspect * half_height;
        origin = lookfrom;
        w = unit_vector(lookfrom - lookat); // lookfrom - lookat is the direction of the camera
        u = unit_vector(cross(vup, w)); // right direction
        v = cross(w, u); // up direction
        lower_left_corner = origin - half_width * focus_dist * u - half_height * focus_dist * v - focus_dist * w; // lower left corner of the screen
        horizontal = 2 * half_width * focus_dist * u; // horizontal length of the screen
        vertical = 2 * half_height * focus_dist * v; // vertical length of the screen
        time0 = t0;
        time1 = t1;

        for (int i = 0; i < num_lens; ++i) {
            float lens_x = random_double(-1, 1);
            float lens_y = random_double(-1, 1);
            float lens_z = random_double(1, 10);
            float lens_radius = random_double(0.1, 0.5);
            lens_positions.push_back(vec3(lens_x, lens_y, lens_z));
            lens_radii.push_back(lens_radius);
            lens_refractive_indices.push_back(random_double(1.0, 2.0));
            lens_reflectances.push_back(random_double(0.5, 1.0));
        }
    }

    ray get_ray(float s, float t) {
        vec3 rd = lens_radius * random_in_unit_disk(); // random sample in the lens, for depth of field
        vec3 offset = u * rd.x() + v * rd.y();
        ray primary_ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, random_double(time0, time1)); // random time for motion blur

        ray r = primary_ray;
        // for (int i = 0; i < lens_positions.size(); i++) {
        //     // Iterate through the lens system
        //     if (hit_lens(r, lens_positions[i], lens_radii[i])) {
        //         // If the ray hits the lens, refract it
        //         vec3 outward_normal = unit_vector(lens_positions[i] - r.origin());
        //         float ni_over_nt = 1.0 / lens_refractive_indices[i];
        //         if (dot(r.direction(), outward_normal) > 0) {
        //             // If the ray is inside the lens
        //             outward_normal = -outward_normal;
        //             ni_over_nt = lens_refractive_indices[i];
        //         }

        //         vec3 refracted;
        //         if (refract(r.direction(), outward_normal, ni_over_nt, refracted)) {
        //             r = ray(r.origin(), refracted, r.time());
        //         } else {
        //             r = ray(r.origin(), reflect(r.direction(), outward_normal), r.time());
        //         }
        //     }
        // }

        // Apply lens effects in order
        apply_lens_distortion(r);
        apply_ghosting(r);
        if (is_light_source_visible(r)) {
            apply_lens_flare(r);
        }

        // Return the ray from the camera to the screen
        return r;
    }

private:
    bool hit_lens(const ray& r, const vec3& lens_position, float lens_radius) const {
        vec3 oc = r.origin() - lens_position;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - lens_radius * lens_radius;
        float discriminant = b * b - a * c;
        return (discriminant > 0);
    }

    bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) const {
        vec3 uv = unit_vector(v);
        float dt = dot(uv, n);
        float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
        if (discriminant > 0) {
            refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
            return true;
        } else
            return false;
    }

    vec3 reflect(const vec3& v, const vec3& n) const {
        return v - 2 * dot(v, n) * n;
    }

    void apply_lens_flare(ray& r) {
        // Simplified lens flare effect based on the angle to the light source
        float flare_intensity = 0.2f; // Adjusted this value for desired intensity
        r.direction() = r.direction() + flare_intensity * vec3(random_double(), random_double(), random_double());
    }

    void apply_ghosting(ray& r) {
        // Simplified ghosting effect by adding small random perturbations
        float ghosting_intensity = 0.1f; // Adjusted this value for desired intensity
        r.origin() = r.origin() + ghosting_intensity * vec3(random_double(), random_double(), random_double());
    }

    void apply_lens_distortion(ray& r) {
        // Apply barrel distortion to the ray direction
        float distortion_coefficient = 0.1f; // Adjusted this value for desired distortion
        vec3 direction = r.direction();
        float r2 = dot(direction, direction);
        vec3 distorted_direction = direction * (1.0f + distortion_coefficient * r2);
        r = ray(r.origin(), distorted_direction, r.time());
    }

    bool is_light_source_visible(const ray& r) const {
        // Simplified check for whether the light source is visible in the scene
        return dot(r.direction(), vec3(0, 0, -1)) > 0.9; // Assume light source is at the z-axis
    }

    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lens_radius;
    float time0, time1; // start and end time of the camera
    std::vector<vec3> lens_positions;
    std::vector<float> lens_radii;
    std::vector<float> lens_refractive_indices;
    std::vector<float> lens_reflectances;
};

#endif
