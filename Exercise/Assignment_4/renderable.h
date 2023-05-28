#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "platform.h"

struct RenderableDesc
{
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec2> texCoords;
	std::vector<vec4> colors;
	std::vector<ivec3> triangles;
	std::string material;
	mat4 modelMatrix;
};

class Renderable
{

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Renderable();

	Renderable(const RenderableDesc& desc);

    Renderable(const std::vector<vec3>& _V,
               const std::vector<ivec3>& _T,
               const std::string& material);
	
    Renderable(const std::vector<vec3>& _V,
               const std::vector<ivec3>& _T,
               const std::string& material,
               const mat4& M);
	
    ~Renderable();

    void initTriangleMesh(const std::vector<vec3>& _V,
							const std::vector<vec3>& _N,
							const std::vector<vec4>& _C,
							const std::vector<vec2>& _ST,
							const std::vector<vec3> &_binormal,
							const std::vector<vec3> &_tangent,
							const std::vector<ivec3>& _T,
							const std::string& material,
							const mat4& M);

	bool getVBO(unsigned int& idx) const;

	bool getIBO(unsigned int& idx) const;

    mat4& getModelMatrix();

    void setModelMatrix(const mat4& M);

    std::string& getMaterial();

    void setMaterial(const std::string& mId);

    void updateVerticesAndNormals(const std::vector<vec3>& _V,
                                    const std::vector<vec3>& _N);

    void updateColors(const std::vector<vec4>& _C);

    void draw();
	
protected:

	void drawTriangleMesh();

	void drawLineList();

    unsigned int mVbo;

    unsigned int mIbo;

    mat4 mModelMatrix;

    std::string mMaterial;

    unsigned int mNumVertices;

	unsigned short mNumIndicesPerElement;

    unsigned int mNumElements;

    unsigned int mVertexSize;

};

#endif // MESH_H
