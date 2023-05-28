#ifndef RENDERER_H
#define RENDERER_H

#include "platform.h"

class Camera;
class Light;
struct LightDesc;
class Renderable;
class ShaderProgram;
class Surface;
struct SurfaceDesc;

typedef std::map<std::string, Renderable*> RenderableMap;
typedef std::pair<std::string, Renderable*> NamedRenderable;
typedef std::map<std::string, Surface*> MaterialMap;
typedef std::pair<std::string, Surface*> NamedMaterial;
typedef std::map<std::string, Light*> LightMap;
typedef std::pair<std::string, Light*> NamedLight;

class Renderer
{

public:

    //! needed for eigen
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    //! constructor
    Renderer();

    //! destructor
    ~Renderer();

    //! initialization of gl/glew
    bool init(unsigned int width,
                unsigned int height,
                const std::string& shaderpath,
                unsigned int numSamples = 1);

    //! call this when window resizes
    void resize(int x, int y);
	
	//! add a renderable
	bool addRenderable(const std::string& name,
						const std::vector<vec3>& _V,
						const std::vector<ivec3>& _T,
						const std::string& material);

    //! add a renderable
    bool addRenderable(const std::string& name,
                        const std::vector<vec3>& _V,
                        const std::vector<ivec3>& _T,
                        const std::string& material,
                        const mat4& M);

    //! modify the renderable
    Renderable* getPtRenderable(const std::string& name);

    //! remove renderable (set entry in the vector to NULL after deletion)
    bool removeRenderable(const std::string& name);

    //! add a surface by a surface description
    bool addSurface(const std::string& name, const SurfaceDesc& desc);

    //! get a manipulator for surface
    Surface* getPtSurface(const std::string& name);

    //! remove surface
    bool removeSurface(const std::string& name);

    //! add a lightsource to scene
    bool addLight(const std::string& name, const LightDesc& desc);

    //! get pointer to lightsource
    Light* getPtLight(const std::string& name);

    //! remove light
    bool removeLight(const std::string& name);

	//! set the clear color
	void setClearColor(const vec4& color);

    //! clear all renderables
    void clearRenderables();

    //! rendercall (contains passes)
    void render(Camera* camera);

protected:

    //! prepare engine supported shader
    bool prepareShader(const std::string& shaderpath);
	
    //! acceleration structure - sorting renderables by material
    std::vector< std::vector<Renderable*> > mRenderTree;

    //! renderable storage
    RenderableMap mRenderables;

    //! material storage
    MaterialMap mMaterials;

    //! light source storage
    LightMap mLights;

    //! the shader used by the surfaces
    std::vector<ShaderProgram*> mShader;
};

#endif // RENDERER_H
