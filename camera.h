#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
#include "random.h"

vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0*vec3(random_double(),random_double(),0) - vec3(1,1,0);
    } while (dot(p,p) >= 1.0);
    return p;
}

class camera {
    public:
        camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focus_dist) {
            // vfov is top to bottom in degrees
            lens_radius = aperture / 2;//aperture is the diameter of the lens
            float theta = vfov*M_PI/180;//convert to radians
            float half_height = tan(theta/2);
            float half_width = aspect * half_height;
            origin = lookfrom;
            w = unit_vector(lookfrom - lookat);//lookfrom - lookat is the direction of the camera
            u = unit_vector(cross(vup, w));//right direction
            v = cross(w, u);//up direction
            lower_left_corner = origin  - half_width*focus_dist*u -half_height*focus_dist*v - focus_dist*w;//lower left corner of the screen
            horizontal = 2*half_width*focus_dist*u;//horizontal length of the screen
            vertical = 2*half_height*focus_dist*v;//vertical length of the screen
        }

    
        ray get_ray(float s, float t) {
            vec3 rd = lens_radius*random_in_unit_disk();//random sample in the lens
            vec3 offset = u * rd.x() + v * rd.y();

            //return the ray from the camera to the screen
            return ray(origin + offset, lower_left_corner + s*horizontal + t*vertical - origin - offset);
        }

        vec3 origin;
        vec3 lower_left_corner;
        vec3 horizontal;
        vec3 vertical;
        vec3 u, v, w;
        float lens_radius;
};


#endif