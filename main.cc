#include <iostream>
#include "sphere.h"
#include "dynamic_sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "random.h"
#include "aabb.h"
#include "bvh.h"
#include "sah.h"


vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, MAXFLOAT, rec)) {
        ray scattered;
        vec3 attenuation;
        //compute color according to the material properties
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
             return attenuation*color(scattered, world, depth+1);
        }
        else {//if the ray is scattered too many times, return black
            return vec3(0,0,0);
        }
    }
    else {//if the ray does not hit an object, return the background color
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
    }
}

//random scene generator, returns a list of hitable objects, including a ground plane, 3 large spheres, and 600 small spheres
hitable *random_scene() {
    int n = 600;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random_double();
            vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());
            if ((center-vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    // list[i++] = new sphere(
                    //     center, 0.2,
                    //     new lambertian(vec3(random_double()*random_double(),
                    //                         random_double()*random_double(),
                    //                         random_double()*random_double()))
                    // );
                    //dynamic sphere
                    auto center1 = center + vec3(0, random_double(0,.5), 0);
                    list[i++] = new dynamic_sphere(
                       center, center1, 0.0, 1.0, 0.5, new lambertian(vec3(random_double()*random_double(),
                                            random_double()*random_double(),
                                            random_double()*random_double()))
                    );
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(
                        center, 0.5,
                        new metal(vec3(0.5*(1 + random_double()),
                                       0.5*(1 + random_double()),
                                       0.5*(1 + random_double())),
                                  0.5*random_double())
                    );
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.5, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(-1, 1, 0), 1.0, new dielectric(1.5));//glass big sphere
    list[i++] = new sphere(vec3(-8, 1, 0), 1.0, new lambertian(vec3(0.2, 0.1, 0.1)));//diffuse big sphere
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.3));//metal big sphere

    //construct the bounding volume hierarchy
    return new bvh_node(list, i, 0.0, 1.0);
    // return new sah_node(list, i, 0.0, 1.0);
    // return new hitable_list(list,i);

}


int main() {
    //image size
    int nx = 960;
    int ny = 540;
    int ns = 10;//number of samples per pixel
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";
    hitable *world = random_scene();
    //view point
    vec3 lookfrom(13,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 20.0;//for depth of field
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus, 0.0, 1.0);

    for (int j = ny-1; j >= 0; j--) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < nx; i++) {
            
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + random_double()) / float(nx);
                float v = float(j + random_double()) / float(ny);
                ray r = cam.get_ray(u, v);
                col += color(r, world,0);
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
    std::cerr << "\nDone.\n";


    return 0;
}