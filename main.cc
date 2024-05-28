#include <iostream>
#include "sphere.h"
#include "rect.h"
#include "box.h"
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
            // float pdf;
            // auto on_light = vec3(213, 554, 227);
            // auto to_light = on_light - rec.p;
            // auto distance_squared = to_light.squared_length();
            // to_light.make_unit_vector();
            // if (dot(to_light, rec.normal) < 0)
            //     return emitted;
            // float light_area = 156*156;
            // float light_cosine = fabs(to_light.y());
            // if (light_cosine < 0.000001)
            //     return emitted;
            // pdf = distance_squared / (light_cosine * light_area);
            // scattered = ray(rec.p, to_light, r.time());

            // float material_pdf = rec.mat_ptr->pdf_value(r, rec, scattered);
            // float weigihted_pdf = material_pdf / pdf;
            // return emitted + attenuation*color(scattered, background, world, depth+1) * weigihted_pdf;
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
    auto moon_normal_map = new image_texture("external/moonnormal.png");
    auto moon_surface = new lambertian(moon_texture);
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, moon_surface);//moon big sphere
    list[i++] = new sphere(vec3(-8, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.3));//metal big sphere


    //add light
    // auto diffuse_light = new light(new solid_color(vec3(4,4,4)));
    // auto light = new xz_rect(0,2, 0, 2, 2, diffuse_light);
    // list[i++] = light;


    //construct the bounding volume hierarchy
    return new bvh_node(list, i, 0.0, 1.0);
    // return new sah_node(list, i, 0.0, 1.0);
    // return new hitable_list(list,i);

}

hitable *cornell_box() {
    hitable **list = new hitable*[20];
    int i = 0;
    material *red = new lambertian(new solid_color(vec3(0.65, 0.05, 0.05)));
    material *white = new lambertian(new solid_color(vec3(0.73, 0.73, 0.73)));
    material *green = new lambertian(new solid_color(vec3(0.12, 0.45, 0.15)));
    
    material *light_area = new light(new solid_color(vec3(15, 15, 15)));

    list[i++] = new yz_rect(0, 555, 0, 555, 555, green);
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(200, 356, 200, 359, 554, light_area);
    list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
    list[i++] = new xz_rect(0, 555, 0, 555, 555, white);
    list[i++] = new xy_rect(0, 555, 0, 555, 555, white);

    //generate hundreds of random spheres of different materials
    // int n = 10;
    // for (int j = 0; j < n; j++) {
    //     list[i++] = new sphere(vec3(165*random_double(), 165*random_double(), 165*random_double()), 100,
    //                            new lambertian(new solid_color(vec3(random_double()*random_double(),
    //                                                               random_double()*random_double(),
    //                                                               random_double()*random_double()))));
    // }

    //list[i++] = new box(vec3(130, 0, 65), vec3(295, 165, 230), white);
    //translate and rotate
    //list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    //list[i++] = new box(vec3(265, 0, 295), vec3(430, 330, 460), white);
    //translate and rotate
    // list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    auto moon_texture = new image_texture("external/moon.jpg");
    auto moon_normal_map = new image_texture("external/moonnormal.png");
    auto moon_surface = new lambertian(moon_texture, moon_normal_map);
    list[i++] = new sphere(vec3(250, 250, 250), 200, moon_surface);//moon big sphere
    //move the moon big sphere to the top of the left box
    



    // return new bvh_node(list, i, 0.0, 1.0);

    //for time comparison
    return new hitable_list(list,i);
}




int main() {
   
    int nx = 800;
    int ny = 800;
    int ns = 300;//number of samples per pixel
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    hitable *world = cornell_box();
    //view point for cornell box
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;//for depth of field
    float aperture =0.0;
    camera cam(lookfrom, lookat, vec3(0,1,0), 40, float(nx)/float(ny), aperture, dist_to_focus, 0.0, 1.0);


    // hitable *world = random_scene();

    // //view point
    // vec3 lookfrom(11,2,3);
    // vec3 lookat(0,0,0);
    // float dist_to_focus = 10.0;//for depth of field
    // float aperture = 0.1;

    // camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus, 0.0, 1.0);

    for (int j = ny-1; j >= 0; j--) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < nx; i++) {
            
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + random_double()) / float(nx);
                float v = float(j + random_double()) / float(ny);
                ray r = cam.get_ray(u, v);
                auto background = vec3(0.5,0.7,1.0);//set background color
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