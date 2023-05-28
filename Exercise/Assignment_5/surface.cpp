#include "surface.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

Surface::Surface(const SurfaceDesc &desc)
{
    setPhongMaterial(PhongMaterial(desc.ambient, desc.diffuse, desc.specular, desc.shine));

    mShaderType = desc.shaderType;
}

Surface::~Surface()
{
	//
}

PhongMaterial& Surface::getPhongMaterial()
{
    return mPhongMaterial;
}

void Surface::setPhongMaterial(const PhongMaterial& pm)
{
    mPhongMaterial = pm;
}

SurfaceDesc::ShaderType& Surface::getShaderType()
{
    return mShaderType;
}
