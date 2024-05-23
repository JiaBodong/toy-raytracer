#ifndef TEXTURE_H
#define TEXTURE_H

#include "vec3.h"
#include "image_loader.h"

// Custom implementation of the clamp function
template <typename T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

class texture {
public:
    virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class solid_color : public texture {
public:
    solid_color() {}
    solid_color(vec3 c) : color_value(c) {}

    solid_color(float red, float green, float blue)
        : solid_color(vec3(red, green, blue)) {}

    virtual vec3 value(float u, float v, const vec3& p) const override {
        return color_value;
    }

private:
    vec3 color_value;
};

//convention function for checker texture
class checker_texture : public texture {
public:
    checker_texture() {}
    checker_texture(texture *t0, texture *t1) : even(t0), odd(t1) {}

    virtual vec3 value(float u, float v, const vec3& p) const override {
        float sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
        if (sines < 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }

    texture *odd;
    texture *even;
};


class image_texture : public texture {
public:
    const static int bytes_per_pixel = 3;

    image_texture() {data = nullptr; nx = ny = 0; bytes_per_scanline = 0;}
    image_texture(const char *filename) {
        auto components_per_pixel = bytes_per_pixel;
        data = stbi_load(filename, &nx, &ny, &components_per_pixel, components_per_pixel);

        if (!data) {
            std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
            nx = ny = 0;
        }

        bytes_per_scanline = bytes_per_pixel * nx;
    }

    ~image_texture() {
        delete data;
    }

    virtual vec3 value(float u, float v, const vec3& p) const override {
        //if no data, return cyan as a debugging aid
        if (data == nullptr)
            return vec3(0, 1, 1);

        //clamp input texture coordinates to [0,1] x [1,0]
        u = clamp(u, 0.0f, 1.0f);
        v = 1.0f - clamp(v, 0.0f, 1.0f); //flip V to image coordinates

        int i = static_cast<int>(u*nx);
        int j = static_cast<int>(v*ny);

        //clamp integer mapping, since actual coordinates should be less than 1.0
        if (i >= nx) i = nx-1;
        if (j >= ny) j = ny-1;

        const auto color_scale = 1.0f/255.0f;
        auto pixel = data + j*bytes_per_scanline + i*bytes_per_pixel;

        return vec3(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
    }

private:
    unsigned char *data;
    int nx, ny;
    int bytes_per_scanline;
};

#endif