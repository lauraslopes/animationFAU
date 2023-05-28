#ifndef PHONGMATERIAL_H
#define PHONGMATERIAL_H

#include "platform.h"

class PhongMaterial
{

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    PhongMaterial();

    PhongMaterial(const vec4& diff);

    PhongMaterial(const vec4& amb,
                  const vec4& diff,
                  const vec4& spec,
                  float shine);

    PhongMaterial(const PhongMaterial& other);

    ~PhongMaterial();

    void operator=(const PhongMaterial& other);

    vec4 ambient, diffuse, specular;

    REAL shininess;
};

#endif
