#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include "sphere.h"
#include "rect.h"
#include "mesh.h"
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
#include "pdf.h"

vec3 color(const ray& r,const vec3& background, hitable *world, hitable** light, int depth) {
    hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        float pdf;
        if (depth < 50 && (rec.mat_ptr->scatter(r, rec, attenuation, scattered,pdf))) {
            
            //light sampling
            // hitable_pdf light_pdf(light, rec.p);
            // scattered = ray(rec.p, light_pdf.generate(), r.time());
            // pdf = light_pdf.value(scattered.direction());

            // //multiple importance sampling(2 light sources)
            // auto p1 = new hitable_pdf(light,0, rec.p);
            // auto p2 = new hitable_pdf(light,1, rec.p);
            // auto p3 = new cosine_pdf(rec.normal);
            // mixture_pdf p(p1, p2, p3);
            // scattered = ray(rec.p, p.generate(), r.time());
            // pdf = p.value(scattered.direction());

            // return emitted + attenuation*rec.mat_ptr->scattering_pdf(r, rec, scattered)*color(scattered, background, world,light, depth+1)/pdf;
            return emitted + attenuation*color(scattered, background, world,light, depth+1);
        } else {
            return emitted;
        }
    } else {
        return background;
    }
}



//random scene generator, returns a list of hitable objects, including a ground plane, 3 large spheres, and 600 small spheres
hitable *random_scene() {
    int n = 60000;
    hitable **list = new hitable*[n+1];

    //tar color ground
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(new solid_color(vec3(0.2, 0.3, 0.3))));

    int i = 1;
    // for (int a = -11; a < 11; a++) {
    //     for (int b = -11; b < 11; b++) {
    //         float choose_mat = random_double();
    //         vec3 center(a+0.9*random_double(),0.2,b+0.9*random_double());
    //         if ((center-vec3(4,0.2,0)).length() > 0.9) {
    //             if (choose_mat < 0.8) {  // diffuse
    //                 // list[i++] = new sphere(
    //                 //     center, 0.2,
    //                 //     new lambertian(vec3(random_double()*random_double(),
    //                 //                         random_double()*random_double(),
    //                 //                         random_double()*random_double()))
    //                 // );
    //                 //dynamic sphere
    //                 auto center1 = center + vec3(0, random_double(0,.5), 0);
    //                 list[i++] = new dynamic_sphere(
    //                    center, center1, 0.0, 1.0, 0.3, new lambertian(vec3(random_double()*random_double(),
    //                                         random_double()*random_double(),
    //                                         random_double()*random_double()))
    //                 );
    //             }
    //             else if (choose_mat < 0.95) { // metal
    //                 list[i++] = new sphere(
    //                     center, 0.3,
    //                     new metal(vec3(0.5*(1 + random_double()),
    //                                    0.5*(1 + random_double()),
    //                                    0.5*(1 + random_double())),
    //                               0.5*random_double())
    //                 );
    //             }
    //             else {  // glass
    //                 list[i++] = new sphere(center, 0.3, new dielectric(1.5));
    //             }
    //         }
    //     }
    // }

    // list[i++] = new sphere(vec3(-1, 1, 0), 1.0, new dielectric(1.5));//glass big sphere
    // // list[i++] = new sphere(vec3(-8, 1, 0), 1.0, new lambertian(vec3(0.2, 0.1, 0.1)));//diffuse big sphere
    
    // list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.5, 0.8, 0.3), 0.3));//metal big sphere
    // list[i++] = new sphere(vec3(-8, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.3));//metal big sphere

    material *car_mat = new lambertian(new solid_color(vec3(0.62, 0.62, 0.62)));
    //car body
    mesh *car = new mesh("external/car.obj", car_mat, 1, vec3(0, 0, 0));
  
    for (int j = 0; j < car->triangles.size(); j++) {
        list[i++] = car->triangles[j];
    }
   
    material *light_mat = new light(new solid_color(vec3(12, 12, 12)));
    mesh *light = new mesh("external/strange_light.obj", light_mat, 0.3f, vec3(18, 0, 0));
    for (int j = 0; j < light->triangles.size(); j++) {
        list[i++] = light->triangles[j];
    }


    //construct the bounding volume hierarchy
    return new bvh_node(list, i, 0.0, 1.0);
    //return new sah_node(list, i, 0.0, 1.0);
    //return new hitable_list(list,i);

}

hitable *cornell_box() {
    hitable **list = new hitable*[20];
    int i = 0;
    material *red = new lambertian(new solid_color(vec3(0.65, 0.05, 0.05)));
    material *white = new lambertian(new solid_color(vec3(0.73, 0.73, 0.73)));
    material *green = new lambertian(new solid_color(vec3(0.12, 0.45, 0.15)));

    material *light_area = new light(new solid_color(vec3(5, 5, 5)));

    list[i++] = new yz_rect(0, 555, 0, 555, 555, green);
    list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
    list[i++] = new xz_rect(200, 356, 200, 359, 554, light_area);
   
    // list[i++] = new yz_rect(200, 356, 200, 359, 0, light_area);
     //another light,same as xz_rect, on the left corner on the floor
    list[i++] = new xz_rect(400, 500, 400, 500, 554, light_area);

    // std::vector<triangle*> tris;
    // //build a Three-dimensional triangle light composed of triangles(not rectangle) (x,z,y)
    // tris.push_back(new triangle(vec3(0, 0, 0), vec3(100, 0, 0), vec3(100, 0, 100), light_area));
    // tris.push_back(new triangle(vec3(0, 0, 0), vec3(100, 0, 100), vec3(0, 0, 100), light_area));
    // tris.push_back(new triangle(vec3(0,0,0),vec3(50,50,50),vec3(100,0,100),light_area));
    // tris.push_back(new triangle(vec3(0,0,0),vec3(50,50,50),vec3(0,0,100),light_area));
    // tris.push_back(new triangle(vec3(100,0,0),vec3(50,50,50),vec3(100,0,100),light_area));
    // tris.push_back(new triangle(vec3(100,0,0),vec3(50,50,50),vec3(0,0,0),light_area));
    // tris.push_back(new triangle(vec3(0,0,100),vec3(50,50,50),vec3(100,0,100),light_area));
    // list[i++] = new mesh(tris);


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

    // list[i++] = new box(vec3(265, 0, 295), vec3(430, 330, 460), white);
    //translate and rotate
    //list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));

    // //big moon sphere
    // auto moon_texture = new image_texture("external/moon.jpg");
    // auto moon_normal_map = new image_texture("external/moonnormal.png");
    // auto moon_surface = new lambertian(moon_texture, moon_normal_map);
    // list[i++] = new sphere(vec3(250, 250, 250), 200, moon_surface);//moon big sphere
  
    

    //return new bvh_node(list, i, 0.0, 1.0);
    //return new sah_node(list, i, 0.0, 1.0);
    //for time comparison
    return new hitable_list(list,i);
}




int main() {
   
    //time record
    auto start = std::chrono::high_resolution_clock::now();

    int nx = 400;
    int ny = 400;
    int ns = 150;//number of samples per pixel
    std::cout << "P3\n" << nx << " " << ny << "\n255\n";

    //multiple light sources
    material *light_area = new light(new solid_color(vec3(5, 5, 5)));
    hitable *light[2];
    light[0] = new xz_rect(400, 500, 400, 500, 554, light_area);
    light[1] = new xz_rect(200, 356, 200, 359, 554, light_area);
  
    // hitable *world = cornell_box();
    // //view point for cornell box
    // vec3 lookfrom(278, 278, -800);
    // vec3 lookat(278, 278, 0);
    // float dist_to_focus = 10.0;//for depth of field
    // float aperture =0.0;

    // camera cam(lookfrom, lookat, vec3(0,1,0), 40, float(nx)/float(ny), aperture, dist_to_focus, 0.0, 1.0,5);


    hitable *world = random_scene();

    vec3 lookfrom(100, 40, 100);
    vec3 lookat(2, 2, 2);
    float dist_to_focus = 50;//for depth of field
    float aperture = 0.1;

    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus, 0.0, 1.0,10);

    for (int j = ny-1; j >= 0; j--) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < nx; i++) {
            
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + random_double()) / float(nx);
                float v = float(j + random_double()) / float(ny);
                ray r = cam.get_ray(u, v);
                //set background color to black,but not pure black
                //vec3 background(0.05, 0.02, 0.1);
                //set background color to sky blue
                vec3 background(0.7, 0.8, 1.0);
                col += color(r,background, world,light,0);
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
    //time record
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cerr << "Time taken: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}