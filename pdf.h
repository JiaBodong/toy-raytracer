#ifndef PDF_H
#define PDF_H

#include "vec3.h"
//PDF management class
class pdf {
    public:
        virtual float value(const vec3& direction) const = 0;
        virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf {
    public:
        cosine_pdf(const vec3& w) { uvw.build_from_w(w); }
        virtual float value(const vec3& direction) const {
            auto cosine = dot(unit_vector(direction), uvw.w());
            return (cosine <= 0) ? 0 : cosine / M_PI;
        }
        virtual vec3 generate() const {
            return uvw.local(random_cosine_direction());
        }
        onb uvw;
};

class hitable_pdf : public pdf {
    public:
        hitable_pdf(hitable **p, int pos,const vec3& origin) : ptr(p), o(origin),index(pos) {}
        virtual float value(const vec3& direction) const {
            return ptr[index]->pdf_value(o, direction);
        }
        virtual vec3 generate() const {
            return ptr[index]->random(o);
        }
        vec3 o;
        int index;
        hitable **ptr;
};

class mixture_pdf : public pdf {
    public:
        mixture_pdf(pdf *p0, pdf *p1, pdf *p2) { p[0] = p0; p[1] = p1; p[2] = p2;}
        virtual float value(const vec3& direction) const {
            return 0.3f * p[0]->value(direction) + 0.3f * p[1]->value(direction) + 0.4f * p[2]->value(direction);
        }
        virtual vec3 generate() const {
            if (drand48() < 0.3f)
                return p[0]->generate();
            else if (drand48() < 0.6f)
                return p[1]->generate();
            else
                return p[2]->generate();
        }
        pdf *p[3];
};

#endif


