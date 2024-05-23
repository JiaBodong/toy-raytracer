#include <iostream>
#include "sphere.h"
#include "rect.h"
#include "dynamic_sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "random.h"
#include "aabb.h"
#include "bvh.h"
#include "sah.h"


vec3 color(const ray& r,const vec3& background, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return emitted + attenuation*color(scattered, background, world, depth+1);
        } else {
            return emitted;
        }
    } else {
        return background;
    }
}

//random scene generator, returns a list of hitable objects, including a ground plane, 3 large spheres, and 600 small spheres
hitable *random_scene() {
    int n = 600;
    hitable **list = new hitable*[n+1];

    auto checker = new checker_texture(new solid_color(vec3(0.2, 0.3, 0.1)), new solid_color(vec3(0.9, 0.9, 0.9)));
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(checker));
    // list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));

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
                       center, center1, 0.0, 1.0, 0.3, new lambertian(vec3(random_double()*random_double(),
                                            random_double()*random_double(),
                                            random_double()*random_double()))
                    );
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(
                        center, 0.3,
                        new metal(vec3(0.5*(1 + random_double()),
                                       0.5*(1 + random_double()),
                                       0.5*(1 + random_double())),
                                  0.5*random_double())
                    );
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.3, new dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(-1, 1, 0), 1.0, new dielectric(1.5));//glass big sphere
    // list[i++] = new sphere(vec3(-8, 1, 0), 1.0, new lambertian(vec3(0.2, 0.1, 0.1)));//diffuse big sphere
    auto moon_texture = new image_texture("external/moon.jpg");
    auto moon_surface = new lambertian(moon_texture);
    list[i++] = new sphere(vec3(-8, 1, 0), 1.0, moon_surface);//moon big sphere
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.3));//metal big sphere


    //add light
    auto diffuse_light = new light(new solid_color(vec3(4,4,4)));
    auto light = new xz_rect(0,2, 0, 2, 2, diffuse_light);
    list[i++] = light;


    //construct the bounding volume hierarchy
    return new bvh_node(list, i, 0.0, 1.0);
    // return new sah_node(list, i, 0.0, 1.0);
    // return new hitable_list(list,i);

}

hitable *cornell_box() {
    hitable **list = new hitable*[8];
    int i = 0;
    material *red = new lambertian(new solid_color(vec3(0.65, 0.05, 0.05)));
    material *white = new lambertian(new solid_color(vec3(0.73, 0.73, 0.73)));
    material *green = new lambertian(new solid_color(vec3(0.12, 0.45, 0.15)));
    material *light = new light(new solid_color(vec3(15, 15, 15)));

    list[i++] = new yz_rect(0, 555, 0, 555, 555, green);
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new xz_rect(0, 555, 0, 555, 555, white);
    list[i++] = new xy_rect(0, 555, 0, 555, 555, white);

    list[i++] = new box(vec3(130, 0, 65), vec3(295, 165, 230), white);
    list[i++] = new box(vec3(265, 0, 295), vec3(430, 330, 460), white);

    return new bvh_node(list, i, 0.0, 1.0);
}




int main() {
   
    int nx = 960;
    int ny = 540;
    int ns = 100;//number of samples per pixel
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    hitable *world = cornell_box();
    // hitable *world = random_scene();

    //view point
    vec3 lookfrom(11,2,3);
    vec3 lookat(0,0,0);
    float dist_to_focus = 15.0;//for depth of field
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
                auto background = vec3(0,0,0);//set background color
                col += color(r,background, world,0);
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