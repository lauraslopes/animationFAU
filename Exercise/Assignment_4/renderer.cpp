#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "renderable.h"
#include "surface.h"
#include "shaderprogram.h"
#include "phongmaterial.h"
#include "matrixutils.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

Renderer::Renderer() 
{
    mRenderTree.clear();
    mRenderables.clear();
    mMaterials.clear();
    mLights.clear();
    mShader.clear();
}

Renderer::~Renderer()
{
    for(unsigned int i = 0; i < mShader.size(); ++i)
    {
        SAFE_DELETE(mShader[i])
    }
    mShader.clear();

	for(LightMap::iterator it = mLights.begin();
		it != mLights.end();
		++it)
	{
		SAFE_DELETE(it->second);
	}
	mLights.clear();

	for(MaterialMap::iterator it = mMaterials.begin();
		it != mMaterials.end();
		++it)
	{
		SAFE_DELETE(it->second);
	}
	mMaterials.clear();

	clearRenderables();
	mRenderTree.clear();
}

bool Renderer::init(unsigned int width, unsigned int height,
                        const std::string& shaderpath,
                        unsigned int numSamples)
{
    // init glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    // init opengl state
    glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glClearStencil(0);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glReadBuffer(GL_BACK);
    glDrawBuffer(GL_BACK);
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFFFFFFFF);
    glStencilFunc(GL_EQUAL, 0x00000000, 0x00000001);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DITHER);
	
    // create all shader related modules
    if(glewIsSupported("GL_VERSION_2_0"))
    {
        if(!prepareShader(shaderpath))
        {
            LOG("shader preparation failed");
            return false;
        }
    }
    else
    {
        // no shader support
        LOG("your system does not support open gl shader");
        return false;
    }

    // add one default material
    SurfaceDesc desc;
    addSurface("defaultMat", desc);

    return true;
}

void Renderer::resize(int x, int y)
{
    // resize screen
    glViewport(0,0,x,y);
}

bool Renderer::addRenderable(const std::string& name,
								const std::vector<vec3>& _V,
								const std::vector<ivec3>& _T,
								const std::string& material)
{
	return addRenderable(name, _V, _T, material, mat4::Identity());
}

bool Renderer::addRenderable(const std::string &name,
                                const std::vector<vec3> &_V,
                                const std::vector<ivec3> &_T,
                                const std::string& material,
                                const mat4& M)
{
    RenderableMap::iterator it1 = mRenderables.find(name);
    if(it1 != mRenderables.end())
    {
        LOG("renderable " << name << " already present");
        return false;
    }

    MaterialMap::iterator it2 = mMaterials.find(material);
    if(it2 == mMaterials.end())
    {
        LOG("material " << material << " not present");
        return false;
    }

    unsigned int shaderid = (unsigned int)((Surface*)it2->second)->getShaderType();
    if(shaderid >= mShader.size())
    {
        LOG("material uses unavailable shader");
        return false;
    }

    Renderable* newEnt = new Renderable(_V, _T, material, M);
	
    mRenderables.insert(NamedRenderable(name, newEnt));
    mRenderTree[shaderid].push_back(newEnt);

    return true;
}

Renderable* Renderer::getPtRenderable(const std::string& name)
{
    RenderableMap::iterator it = mRenderables.find(name);
    if(it != mRenderables.end())
    {
        return it->second;
    }

    return NULL;
}

bool Renderer::removeRenderable(const std::string &name)
{
    RenderableMap::iterator it1 = mRenderables.find(name);

    if(it1 == mRenderables.end())
    {
        LOG("renderable " << name << " not found");
        return false;
    }

    // remove from render tree
    MaterialMap::iterator it2 = mMaterials.find(((Renderable*)it1->second)->getMaterial());
    Surface* removeSurface = it2->second;
    unsigned int shaderid = (unsigned int) removeSurface->getShaderType();
    if(shaderid < mShader.size())
    {
        for(unsigned int i = 0; i < mRenderTree[shaderid].size(); ++i)
        {
            if(mRenderTree[shaderid][i] == it1->second)
                mRenderTree[shaderid].erase(mRenderTree[shaderid].begin()+i);
        }
    }

    // delete, destroy and erase
    delete it1->second;
    it1->second = NULL;
    mRenderables.erase(it1);

    return true;
}

bool Renderer::addSurface(const std::string& name, const SurfaceDesc& desc)
{
    MaterialMap::iterator it = mMaterials.find(name);
    if(it != mMaterials.end())
    {
        LOG("material " << name << " already present");
        return false;
    }

    Surface* newEnt = new Surface(desc);
    mMaterials.insert(NamedMaterial(name, newEnt));
    return true;
}

Surface* Renderer::getPtSurface(const std::string& name)
{
    MaterialMap::iterator it = mMaterials.find(name);
    if(it != mMaterials.end())
    {
        return it->second;
    }
    return NULL;
}

bool Renderer::removeSurface(const std::string &name)
{
    MaterialMap::iterator it = mMaterials.find(name);

    if(it == mMaterials.end())
    {
        LOG("suface " << name << " not found");
        return false;
    }

    delete it->second;
    it->second = NULL;
    mMaterials.erase(it);

    return true;
}

bool Renderer::addLight(const std::string& name, const LightDesc& desc)
{
    if(mLights.size() == 3)
    {
        LOG("maximum number of lights: 3");
        return false;
    }

    LightMap::iterator it = mLights.find(name);
    if(it != mLights.end())
    {
        LOG("lightsource " << name << " already present");
        return false;
    }

    Light* newL = new Light(desc);
    mLights.insert(NamedLight(name, newL));

    return true;
}

Light* Renderer::getPtLight(const std::string& name)
{
    LightMap::iterator it = mLights.find(name);
    if(it != mLights.end())
    {
        return it->second;
    }
    return NULL;
}

bool Renderer::removeLight(const std::string& name)
{
    // TODO: implement after implemented multi light
    if(name == "<hallo welt>")
        LOG("GOOGOOG");

    LOG("TODO: implement me");

    return false;
}

void Renderer::setClearColor(const vec4& color)
{
	glClearColor(color.x(), color.y(), color.z(), color.w());
}

void Renderer::clearRenderables()
{
	for(RenderableMap::iterator it = mRenderables.begin(); it != mRenderables.end(); ++it)
	{
		SAFE_DELETE(it->second);
	}
	mRenderables.clear();
	
	for(unsigned int i = 0; i < mRenderTree.size(); ++i)
	{
		mRenderTree[i].clear();
	}
}

void Renderer::render(Camera* camera)
{
    // render scene to rendertarget
    mat4 view_matrix, proj_matrix;
    camera->getViewMatrix(view_matrix);
    camera->getProjMatrix(proj_matrix);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // for all shaders
    for(unsigned int i = 0; i < mShader.size(); ++i)
    {
        ShaderProgram* shader = NULL;
        shader = mShader[i];

        //bind the shader
        shader->bind();

        // set camera
        glUniformMatrix4fv(shader->uniform("view_matrix"), 1, false, view_matrix.data());
        glUniformMatrix4fv(shader->uniform("proj_matrix"), 1, false, proj_matrix.data());

        // setup lightsources
        if(mLights.size() == 0)
        {
            shader->release();
            continue;
        }

        LightMap::iterator it = mLights.begin();
        Light* currentLight = it->second;

        // get locations
		currentLight->setUniforms(view_matrix, shader->uniform("light_position"), shader->uniform("light_I"));

        // render all renderables for this shader
        for(unsigned int j = 0; j < mRenderTree[i].size(); ++j)
        {
            Renderable* cur = mRenderTree[i][j];

            if(cur == NULL)
                continue;

            // get material
            MaterialMap::iterator it = mMaterials.find(cur->getMaterial());

            if(it == mMaterials.end())
            {
                LOG("material " << cur->getMaterial() << " not found");
                continue;
            }

            Surface* curSurface = it->second;

            // upload uniforms according to shader type
            switch(curSurface->getShaderType())
            {
                // simple phong shader uses only the am, dif, spec material reflectance
            case SurfaceDesc::SHADER_PHONG:                
                glUniform4fv(shader->uniform("ambient"), 1, curSurface->getPhongMaterial().ambient.data());
                glUniform4fv(shader->uniform("diffuse"), 1, curSurface->getPhongMaterial().diffuse.data());
                glUniform4fv(shader->uniform("specular"), 1, curSurface->getPhongMaterial().specular.data());
                glUniform1f(shader->uniform("shine"), curSurface->getPhongMaterial().shininess);
                break;

            default:
                LOG("unknown shader type");
                break;
            }

            // set modelmatrix
            mat4& mM = cur->getModelMatrix();
            glUniformMatrix4fv(shader->uniform("model_matrix"), 1, false, mM.data());

            // ... and the normalmatrix
            mat4 mvM = view_matrix * cur->getModelMatrix();
            mat3 mvM3 = mvM.topLeftCorner<3,3>();
            mat3 nM = mvM3.inverse().transpose();

            glUniformMatrix3fv(shader->uniform("normal_matrix"), 1, false, nM.data());

            // draw
            cur->draw();
        }

        shader->release();
    }
}

bool Renderer::prepareShader(const std::string& shaderpath)
{
    //
    // compile all shader for standard rendering
    //
    mShader.clear();

    // phong shader
    LOG("Phong");
    ShaderProgram* phongshader = new ShaderProgram();
    phongshader->addShaderFromSource(ShaderProgram::SHADER_VERTEX, shaderpath + "phong.vs");
    phongshader->addShaderFromSource(ShaderProgram::SHADER_FRAGMENT, shaderpath + "phong.fs");
    if(!phongshader->link())
    {
        LOG("phongshader linkage error");
        return false;
    }
    mShader.push_back(phongshader);
	
    //
    // init the render tree
    //
    mRenderTree.resize(mShader.size());
    for(unsigned int i = 0; i < mRenderTree.size(); ++i)
    {
        mRenderTree[i].clear();
    }

    std::cerr << "loaded " << mRenderTree.size() << " shader programs\n";

    return true;
}
