#ifndef SURFACE_H
#define SURFACE_H

#include "platform.h"
#include "phongmaterial.h"

class Texture;

struct SurfaceDesc
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum ShaderType
    {
        SHADER_PHONG = 0, // shader for simple per pixel phong lighting
        SHADER_POINTS = 1, // shader for simple point rendering (hack, the w coordinate of ambient color is the radius)
		SHADER_LINES = 2 // shader for line rendering
    };

    vec4 ambient, diffuse, specular;
    REAL shine;
    std::string diffuse_map, normal_map;
    ShaderType shaderType;

    SurfaceDesc()
    {
        REAL h = REAL(0.5);
        ambient = vec4(0,0,0,1);
        diffuse = vec4(h,h,h,1);
        specular = vec4(0,0,0,1);
        shine = REAL(1);
        shaderType = SHADER_PHONG;
    }
};

class Surface
{

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Surface(const SurfaceDesc& desc);

    ~Surface();

    PhongMaterial& getPhongMaterial();

    void setPhongMaterial(const PhongMaterial& material);

    SurfaceDesc::ShaderType& getShaderType();

protected:

    //! total fallback
    PhongMaterial mPhongMaterial;
	
    //! the shader for this surface
    SurfaceDesc::ShaderType mShaderType;
};

#endif 
