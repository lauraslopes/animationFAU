#include "renderable.h"
#include "geomutils.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

Renderable::Renderable() :
	mNumVertices(0), 
	mNumElements(0),
	mNumIndicesPerElement(0),
	mVertexSize(0)
{
	// generate buffers
	glGenBuffers(1, &mVbo);
	glGenBuffers(1, &mIbo);

	mMaterial = "defaultMat";
	mModelMatrix.setIdentity();
}

Renderable::Renderable(const RenderableDesc& desc) :
	mNumVertices(0),
	mNumElements(0),
	mNumIndicesPerElement(0),
	mVertexSize(0)
{
	glGenBuffers(1, &mVbo);
	glGenBuffers(1, &mIbo);

	std::vector<vec2> _texcoord(0);
	std::vector<vec3> _binorm(0), _tangent(0);
	
	initTriangleMesh(desc.vertices, 
						desc.normals,
						desc.colors,
						_texcoord,
						_binorm, 
						_tangent, 
						desc.triangles,
						desc.material, 
						desc.modelMatrix);

}

Renderable::Renderable(const std::vector<vec3> &_V,
                        const std::vector<ivec3> &_T,
                        const std::string& material) : 
	mNumVertices(0), 
	mNumElements(0),
	mNumIndicesPerElement(0),
	mVertexSize(0)
{
	// generate buffers
	glGenBuffers(1, &mVbo);
	glGenBuffers(1, &mIbo);

    // init per vertex normal
    std::vector<vec3> _N;
    computeTriangleMeshNormals(_V,_T,_N);

    // init per vertex color
    std::vector<vec4> _C(0);

    // set per vertex st, binorm, tangent
    std::vector<vec2> _ST(0);
    std::vector<vec3> _binorm(0);
    std::vector<vec3> _tangent(0);

    // init transformation
    mat4 M;
    M.setIdentity();
    initTriangleMesh(_V, _N, _C, _ST, _binorm, _tangent, _T, material, M);
}

Renderable::Renderable(const std::vector<vec3>& _V,
                       const std::vector<ivec3>& _T,
                       const std::string& material,
                       const mat4& M) :
	mNumVertices(0), 
	mNumElements(0),
	mNumIndicesPerElement(0),
	mVertexSize(0)
{
	// generate buffers
	glGenBuffers(1, &mVbo);
	glGenBuffers(1, &mIbo);

    std::vector<vec3> _N;
    computeTriangleMeshNormals(_V,_T,_N);
    std::vector<vec4> _C(0);
    std::vector<vec2> _ST(0);
    std::vector<vec3> _binorm(0);
    std::vector<vec3> _tangent(0);
    initTriangleMesh(_V, _N, _C, _ST, _binorm, _tangent, _T, material, M);
}

Renderable::~Renderable()
{
	mNumVertices = 0;
	mNumElements = 0;
	mNumIndicesPerElement = 0;
	mVertexSize = 0;

    if(glIsBuffer(mVbo))
        glDeleteBuffers(1, &mVbo);

    if(glIsBuffer(mIbo))
        glDeleteBuffers(1, &mIbo);
}

void Renderable::initTriangleMesh(const std::vector<vec3>& _V,
									const std::vector<vec3>& _N,
									const std::vector<vec4>& _C,
									const std::vector<vec2>& _ST,
									const std::vector<vec3>& _binormal,
									const std::vector<vec3>& _tangent,
									const std::vector<ivec3>& _T,
									const std::string& material,
									const mat4& M)
{
    // clear in case of existing data
    mNumVertices = 0;
	mNumElements = 0;
	mNumIndicesPerElement = 0;
	
    // 3 pos,
    // 3 normal,
    // 4 color,
    // 2 st,
    // 3 binorm,
    // 3 tangent
    mVertexSize = 3 + 3 + 4 + 2 + 3 + 3;

    // generate interleaved vertex array
    mNumVertices = _V.size();
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    float* v = new float[_V.size() * mVertexSize];
    for(unsigned int i = 0; i < _V.size(); ++i)
    {
        v[mVertexSize*i+0] = _V[i].x();
        v[mVertexSize*i+1] = _V[i].y();
        v[mVertexSize*i+2] = _V[i].z();
        v[mVertexSize*i+3] = _N[i].x();
        v[mVertexSize*i+4] = _N[i].y();
        v[mVertexSize*i+5] = _N[i].z();
        v[mVertexSize*i+6] = (_C.size() == _V.size()) ? _C[i].x() : 1;
        v[mVertexSize*i+7] = (_C.size() == _V.size()) ? _C[i].y() : 1;
        v[mVertexSize*i+8] = (_C.size() == _V.size()) ? _C[i].z() : 1;
        v[mVertexSize*i+9] = (_C.size() == _V.size()) ? _C[i].w() : 1;
        v[mVertexSize*i+10] = (_ST.size() == _V.size()) ? _ST[i].x() : 0;
        v[mVertexSize*i+11] = (_ST.size() == _V.size()) ? _ST[i].y() : 0;
        v[mVertexSize*i+12] = (_binormal.size() == _V.size()) ? _binormal[i].x() : 0;
        v[mVertexSize*i+13] = (_binormal.size() == _V.size()) ? _binormal[i].y() : 0;
        v[mVertexSize*i+14] = (_binormal.size() == _V.size()) ? _binormal[i].z() : 0;
        v[mVertexSize*i+15] = (_tangent.size() == _V.size()) ? _tangent[i].x() : 0;
        v[mVertexSize*i+16] = (_tangent.size() == _V.size()) ? _tangent[i].y() : 0;
        v[mVertexSize*i+17] = (_tangent.size() == _V.size()) ? _tangent[i].z() : 0;
    }
    glBufferData(GL_ARRAY_BUFFER, _V.size()*mVertexSize*sizeof(float), &v[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    delete [] v;

    // upload index data (if there is)
    if(_T.size() > 0)
    {
        mNumElements = _T.size();
		mNumIndicesPerElement = 3;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
        unsigned int* t = new unsigned int[_T.size()*mNumIndicesPerElement];
        for(unsigned int i = 0; i < _T.size(); ++i)
        {
            t[mNumIndicesPerElement*i] = _T[i].x();
            t[mNumIndicesPerElement*i+1] = _T[i].y();
            t[mNumIndicesPerElement*i+2] = _T[i].z();
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _T.size()*mNumIndicesPerElement*sizeof(unsigned int), &t[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        delete [] t;
    }

    // apply modelmatrix
    mModelMatrix = M;

    // apply material index
    mMaterial = material;
}

bool Renderable::getVBO(unsigned int& idx) const
{
	if(!glIsBuffer(mVbo))
	{
		idx = 0;
		return false;
	}

	idx = mVbo;
	return true;
}

bool Renderable::getIBO(unsigned int& idx) const
{
	if(!glIsBuffer(mIbo))
	{
		idx = 0;
		return false;
	}

	idx = mIbo;
	return true;
}

mat4& Renderable::getModelMatrix()
{
    return mModelMatrix;
}

void Renderable::setModelMatrix(const mat4 &M)
{
    mModelMatrix = M;
}

std::string& Renderable::getMaterial()
{
    return mMaterial;
}

void Renderable::setMaterial(const std::string& material)
{
    mMaterial = material;
}

void Renderable::updateVerticesAndNormals(const std::vector<vec3>& _V,
                                            const std::vector<vec3>& _N)
{
    if((_V.size() != _N.size()) || (_V.size() != mNumVertices))
    {
        LOG("size mismatch");
        return;
    }

    // map buffer
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    float* v = (float*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    // update
    for(unsigned int i = 0; i < _V.size(); ++i)
    {
        v[mVertexSize*i+0] = _V[i].x();
        v[mVertexSize*i+1] = _V[i].y();
        v[mVertexSize*i+2] = _V[i].z();
        v[mVertexSize*i+3] = _N[i].x();
        v[mVertexSize*i+4] = _N[i].y();
        v[mVertexSize*i+5] = _N[i].z();
    }

    // unmap buffer
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderable::updateColors(const std::vector<vec4>& _C)
{
    if(_C.size() != mNumVertices)
    {
        LOG("size mismatch");
        return;
    }

    // map buffer
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    float* v = (float*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    for(unsigned int i = 0; i < _C.size(); ++i)
    {
        v[mVertexSize*i+6] = _C[i].x();
        v[mVertexSize*i+7] = _C[i].y();
        v[mVertexSize*i+8] = _C[i].z();
        v[mVertexSize*i+9] = _C[i].w();
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderable::draw()
{
	switch(mNumIndicesPerElement)
	{
	case 0:
		// this "pseudo" case is handled by triangle drawer (point cloud w/o indices)
		drawTriangleMesh();
		break;

	case 1:
		LOG("native point rendering not implemented yet");
		break;
		
	case 2:
		drawLineList();
		break;

	case 3:
		drawTriangleMesh();
		break;

	case 4:
		LOG("native quad rendering not implemented yet");

	default:
		LOG("number of indices per primitive not specified");
	}
}

void Renderable::drawTriangleMesh()
{
    // bind the object's buffer
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, mVertexSize*sizeof(float), (void*)(0));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, mVertexSize*sizeof(float), (void*)(12));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, mVertexSize*sizeof(float), (void*)(24));

    // make things cleaner
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bool renderElements = (mNumElements > 0);
    if(renderElements)
    {
        // bind indices and draw
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
        glDrawElements(GL_TRIANGLES, mNumIndicesPerElement*mNumElements, GL_UNSIGNED_INT, (void*)(0));

        // unbind the element array also
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
        // bind array buffer
        glDrawArrays(GL_POINTS, 0, mNumVertices);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Renderable::drawLineList()
{
	// bind the object's buffer
	glBindBuffer(GL_ARRAY_BUFFER, mVbo);

	// point to positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, mVertexSize*sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);

	// point to color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, mVertexSize*sizeof(float), (void*)(12));
	glEnableVertexAttribArray(1);
	
	// make things cleaner
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	bool renderElements = (mNumElements > 0);
	if(renderElements)
	{
		// bind indices and draw
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbo);
		glDrawElements(GL_LINES, mNumIndicesPerElement*mNumElements, GL_UNSIGNED_INT, (void*)(0));

		// unbind the element array also
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
	{
		// bind array buffer
		glDrawArrays(GL_POINTS, 0, mNumVertices);
	}

	// disable vertexattribs
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
