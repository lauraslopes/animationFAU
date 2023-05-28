#include "phongmaterial.h"

PhongMaterial::PhongMaterial()
    : shininess(1) {}

PhongMaterial::PhongMaterial(const vec4& diff)
    : diffuse(diff),
      shininess(1) {}

PhongMaterial::PhongMaterial(const vec4& amb,
                             const vec4& diff,
                             const vec4& spec,
                             float shine)
    : ambient(amb),
      diffuse(diff),
      specular(spec),
      shininess(shine) {}

PhongMaterial::PhongMaterial(const PhongMaterial& other)
    : ambient(other.ambient),
      diffuse(other.diffuse),
      specular(other.specular),
      shininess(other.shininess) {}

PhongMaterial::~PhongMaterial() {}

void PhongMaterial::operator=(const PhongMaterial& other)
{
    ambient = other.ambient;
    diffuse = other.diffuse;
    specular = other.specular;
    shininess = other.shininess;
}
