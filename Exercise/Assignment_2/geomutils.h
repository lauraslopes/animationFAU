#ifndef GEOMUTILS_H
#define GEOMUTILS_H

#include "platform.h"
#include "matrixutils.h"

/*! computeTriangleMeshNormals()
 *
 *  \brief  calculates a smooth per vertex normal set (normalized)
 */
static void computeTriangleMeshNormals(const std::vector<vec3>& v, const std::vector<ivec3>& t, std::vector<vec3>& out)
{
    out.resize(v.size());
	for(unsigned int i = 0; i < out.size(); ++i)
	{
		out[i] = vec3(0,0,0);
	}

    for(unsigned int i = 0; i < t.size(); ++i)
    {
        vec3 A = v[t[i][0]];
        vec3 B = v[t[i][1]];
        vec3 C = v[t[i][2]];
        vec3 fn = (B-A).cross(C-A).normalized();
        out[t[i][0]] += fn;
        out[t[i][1]] += fn;
        out[t[i][2]] += fn;
    }

    for(unsigned int j = 0; j < out.size(); ++j)
        out[j].normalize();
}

/*! centerMesh()
 *
 *  \brief  given scattered point data inV, cog is calculated and the
 *          centered data is written back to outV
 */
static vec3 centerMesh(const std::vector<vec3>& inV, std::vector<vec3>& outV)
{
    vec3 cog;
    cog.setZero();

    for(unsigned int i = 0; i < inV.size(); ++i)
    {
        cog += inV[i];
    }
    cog /= (REAL) inV.size();

    outV.resize(inV.size());

    for(unsigned int j = 0; j < outV.size(); ++j)
    {
        outV[j] = inV[j] - cog;
    }

    return cog;
}

// one argument version
static void centerMesh(std::vector<vec3>& inout)
{
    std::vector<vec3> _tmp;
    _tmp = inout;
    inout.clear();
    centerMesh(_tmp, inout);
}

/*! offsetMesh()
 *
 * \brief sub a vector from all vertices
 */
static void offsetMesh(std::vector<vec3>& _V, const vec3& offset)
{
    for(unsigned int i = 0; i < _V.size(); ++i)
    {
        _V[i] -= offset;
    }
}

/*! rotateMesh()
 *
 * \brief rotate vertices by matrix
 */
static void rotateMesh(std::vector<vec3>& _V, const mat3& R)
{
    std::vector<vec3> _tmp;
    _tmp = _V;
    _V.clear();

    for(unsigned int i = 0; i < _tmp.size(); ++i)
    {
        _V.push_back(R * _tmp[i]);
    }
}

/*! scaleMesh()
 *
 * \brief scale vertices by scalar
 */
static void scaleMesh(std::vector<vec3>& inout, REAL scaleFactor)
{
	for(unsigned int i = 0; i < inout.size(); ++i)
		inout[i] *= scaleFactor;
}

/*! getGoodBasis
 */
static void getGoodBasis(const vec3& e0, mat3& B)
{
    // find a vector that is in a good ortho plane
    vec3 fix(0,0,-1);

    if(e0[1] < 0)
        fix = vec3(0,0,1);

    // let
    vec3 e2 = e0.cross(fix);
    e2.normalize();
    vec3 e1 = e2.cross(e0);
    e1.normalize();
    B.setZero();
    B.col(0) = e2;
    B.col(1) = e0;  // normal
    B.col(2) = e1;
}

/*! get some basic primitives

*/
static void createXZPlane(const REAL& size, std::vector<vec3>& _V,
                                std::vector<vec3>& _N,
                                std::vector<vec2>& _ST,
                                std::vector<ivec3>& _T)
{
    _V.clear();
    _N.clear();
    _ST.clear();
    _T.clear();
    REAL s = std::abs(size)*REAL(0.5);
    vec3 n(0,1,0);
    _V.push_back(vec3(-s,0,s));
    _N.push_back(n);
    _ST.push_back(vec2(0,0));
    _V.push_back(vec3(s,0,s));
    _N.push_back(n);
    _ST.push_back(vec2(1,0));
    _V.push_back(vec3(s,0, -s));
    _N.push_back(n);
    _ST.push_back(vec2(1,1));
    _V.push_back(vec3(-s,0, -s));
    _N.push_back(n);
    _ST.push_back(vec2(0,1));
    _T.push_back(ivec3(0,1,2));
    _T.push_back(ivec3(0,2,3));
}

static void createSolidCylinder(const vec3& start,
                                    const vec3& end,
                                    REAL radius,
                                    unsigned int sectionCircle,
                                    std::vector<vec3>& _V,
                                    std::vector<vec3>& _N,
                                    std::vector<vec2>& _ST,
                                    std::vector<ivec3>& _T)
{
    if(sectionCircle < 3)
        sectionCircle = 3;

    if(radius < 0.001)
        radius = REAL(0.001);

    mat3 B;
    vec3 dir = end-start;
    getGoodBasis(dir, B);
    vec3 x = B.col(0);
    vec3 y = B.col(2);

    // create a cylinder
    REAL angleStep = (REAL(2.0*M_PI)) / sectionCircle;

    // one section is a quad, has 2 triangles, each 3 vertices, each 3 komponents sec*2*3*3
    _V.clear();
    _N.clear();
    _ST.clear();
    _T.clear();

    for(unsigned int i = 0; i < sectionCircle; ++i)
    {
        vec3 lowPoint = start + radius * (std::cos(angleStep * i) * x + std::sin(angleStep * i) * y);
        vec3 hiPoint = end + radius * (std::cos(angleStep * i) * x + std::sin(angleStep * i) * y);

        vec3 lowN = std::cos(angleStep * i) * x + std::sin(angleStep * i) * y;
        vec3 hiN = std::cos(angleStep * i) * x + std::sin(angleStep * i) * y;

        // push the hi and low vertices to the mesh
        _V.push_back(lowPoint);
        _V.push_back(hiPoint);
        _N.push_back(lowN);
        _N.push_back(hiN);
        _ST.push_back(vec2(REAL(i)/REAL(sectionCircle),0));
        _ST.push_back(vec2(REAL(i)/REAL(sectionCircle),0));

        // in the first step, done
        if(i == 0)
            continue;

        // add triangles
        ivec3 tri0, tri1;
        unsigned int _i = 2 + (2*i-1);
        tri0.z() = _i - 3;
        tri0.y() = _i;
        tri0.x() = _i - 2;
        tri1.z() = _i - 3;
        tri1.y() = _i - 1;
        tri1.x() = _i;

        _T.push_back(tri0);
        _T.push_back(tri1);
    }

    // insert last 2 faces
    ivec3 tri0, tri1;
    unsigned int _i = _V.size() - 1;
    tri0.z() = _i - 1;
    tri0.y() = 1;
    tri0.x() = _i;
    tri1.z() = _i - 1;
    tri1.y() = 0;
    tri1.x() = 1;
    _T.push_back(tri0);
    _T.push_back(tri1);

    // insert caps /* _V: l0 h0 l1 h1 l2 h2 ... */
    for(unsigned int i = 1; i < sectionCircle-1; ++i)
    {
        ivec3 triL, triH;
        triL.x() = 0;
        triL.y() = 2*i;
        triL.z() = 2*(i+1);
        triH.x() = 1;
        triH.y() = 2*(i+1)+1;
        triH.z() = 2*i+1;
        _T.push_back(triL);
        _T.push_back(triH);
    }
}

static void createSolidIcosphere(const vec3& center, REAL radius,
                                    std::vector<vec3>& _V,
                                    std::vector<vec3>& _N,
                                    std::vector<ivec3>& _T)
{
    if(radius < 0.001)
        radius = REAL(0.001);

    radius /= REAL(2);

    _V.clear();
    _N.clear();
    _T.clear();

    REAL t = (REAL(1) + std::sqrt(REAL(5)) / REAL(2));

    // add vertices
    _V.push_back(vec3(REAL(-1), t, 0) * radius);
    _V.push_back(vec3(REAL(1), t, 0) * radius);
    _V.push_back(vec3(REAL(-1), -t, 0) * radius);
    _V.push_back(vec3(REAL(1), -t, 0) * radius);

    _V.push_back(vec3(0, REAL(-1), t) * radius);
    _V.push_back(vec3(0, REAL(1), t) * radius);
    _V.push_back(vec3(0, REAL(-1), -t) * radius);
    _V.push_back(vec3(0, REAL(1), -t) * radius);

    _V.push_back(vec3(t, 0, REAL(-1)) * radius);
    _V.push_back(vec3(t, 0, REAL(1)) * radius);
    _V.push_back(vec3(-t, 0, REAL(-1)) * radius);
    _V.push_back(vec3(-t, 0, REAL(1)) * radius);

    // add normals
    _N.push_back(vec3(REAL(-1), t, 0).normalized());
    _N.push_back(vec3(REAL(1), t, 0).normalized());
    _N.push_back(vec3(REAL(-1), -t, 0).normalized());
    _N.push_back(vec3(REAL(1), -t, 0).normalized());

    _N.push_back(vec3(0, REAL(-1), t).normalized());
    _N.push_back(vec3(0, REAL(1), t).normalized());
    _N.push_back(vec3(0, REAL(-1), -t).normalized());
    _N.push_back(vec3(0, REAL(1), -t).normalized());

    _N.push_back(vec3(t, 0, REAL(-1)));
    _N.push_back(vec3(t, 0, REAL(1)));
    _N.push_back(vec3(-t, 0, REAL(-1)));
    _N.push_back(vec3(-t, 0, REAL(1)));

    // shift all vertices to match center
    for(unsigned int i = 0; i < _V.size(); ++i)
    {
        _V[i] += center;
    }

    // create triangles
    _T.push_back(ivec3(INDEX(0), INDEX(11), INDEX(5)));
    _T.push_back(ivec3(INDEX(0), INDEX(5), INDEX(1)));
    _T.push_back(ivec3(INDEX(0), INDEX(1), INDEX(7)));
    _T.push_back(ivec3(INDEX(0), INDEX(7), INDEX(10)));
    _T.push_back(ivec3(INDEX(0), INDEX(10), INDEX(11)));

    _T.push_back(ivec3(INDEX(1), INDEX(5), INDEX(9)));
    _T.push_back(ivec3(INDEX(5), INDEX(11), INDEX(4)));
    _T.push_back(ivec3(INDEX(11), INDEX(10), INDEX(2)));
    _T.push_back(ivec3(INDEX(10), INDEX(7), INDEX(6)));
    _T.push_back(ivec3(INDEX(7), INDEX(1), INDEX(8)));

    _T.push_back(ivec3(INDEX(3), INDEX(9), INDEX(4)));
    _T.push_back(ivec3(INDEX(3), INDEX(4), INDEX(2)));
    _T.push_back(ivec3(INDEX(3), INDEX(2), INDEX(6)));
    _T.push_back(ivec3(INDEX(3), INDEX(6), INDEX(8)));
    _T.push_back(ivec3(INDEX(3), INDEX(8), INDEX(9)));

    _T.push_back(ivec3(INDEX(4), INDEX(9), INDEX(5)));
    _T.push_back(ivec3(INDEX(2), INDEX(4), INDEX(11)));
    _T.push_back(ivec3(INDEX(6), INDEX(2), INDEX(10)));
    _T.push_back(ivec3(INDEX(8), INDEX(6), INDEX(7)));
    _T.push_back(ivec3(INDEX(9), INDEX(8), INDEX(1)));
}

static void createSolidCone(const vec3& start,
                       const vec3& tip,
                       REAL radius,
                       unsigned int sectionCircle,
                       std::vector<vec3>& _V,
                       std::vector<vec3>& _N,
                       std::vector<vec2>& _ST,
                       std::vector<ivec3>& _T)
{
    if(sectionCircle < 3)
        sectionCircle = 3;

    if(radius < 0.001)
        radius = REAL(0.001);

    mat3 B;
    vec3 dir = (tip-start).normalized();
    getGoodBasis(dir, B);
    vec3 x = B.col(0);
    vec3 y = B.col(2);

    REAL angleStep = (REAL(2.0*M_PI)) / sectionCircle;

    _V.clear();
    _N.clear();
    _ST.clear();
    _T.clear();

    // insert tip
    _V.push_back(tip);
    _N.push_back(vec3(dir));
    _ST.push_back(vec2(1,1));

    for(unsigned int i = 0; i < sectionCircle; ++i)
    {
        vec3 lowPoint = start + radius * (std::cos(angleStep * i) * x + std::sin(angleStep * i) * y);
        vec3 lowN = std::cos(angleStep * i) * x + std::sin(angleStep * i) * y;

        // push the hi and low vertices to the mesh
        _V.push_back(lowPoint);
        _N.push_back(lowN);
        _ST.push_back(vec2(REAL(i)/REAL(sectionCircle),0));

        // in the first step, done
        if(i == 0)
            continue;

        // add triangles
        ivec3 tri;
        tri.x() = 0;
        tri.y() = i+1;
        tri.z() = i;

        _T.push_back(tri);
    }

    // add last triangle
    ivec3 tri;
    tri.x() = 0;
    tri.y() = 1;
    tri.z() = _V.size()-1;
    _T.push_back(tri);

    // insert cap /* _V: l0 h0 l1 h1 l2 h2 ... */
    for(unsigned int i = 1; i < sectionCircle-1; ++i)
    {
        ivec3 tri;
        tri.x() = 1;
        tri.y() = i+1;
        tri.z() = i+2;
        _T.push_back(tri);
    }
}

// create a lined box from min/max of a AABB
static void createLineBoxAABB(const vec3& minP, const vec3& maxP,
                                std::vector<vec3>& _V,
                                std::vector<ivec2>& _T)
{
    _V.clear();
    _T.clear();

    // corner vertices
	_V.push_back(minP);
	_V.push_back(vec3(minP.x(), minP.y(), maxP.z()));
	_V.push_back(vec3(maxP.x(), minP.y(), maxP.z()));
	_V.push_back(vec3(maxP.x(), minP.y(), minP.z()));
	_V.push_back(vec3(minP.x(), maxP.y(), minP.z()));
	_V.push_back(vec3(minP.x(), maxP.y(), maxP.z()));
	_V.push_back(maxP);
	_V.push_back(vec3(maxP.x(), maxP.y(), minP.z()));

	// topology
	_T.push_back(ivec2(0,1));
	_T.push_back(ivec2(1,2));
	_T.push_back(ivec2(2,3));
	_T.push_back(ivec2(3,0));

	_T.push_back(ivec2(0,4));
	_T.push_back(ivec2(1,5));
	_T.push_back(ivec2(2,6));
	_T.push_back(ivec2(3,7));

	_T.push_back(ivec2(4,5));
	_T.push_back(ivec2(5,6));
	_T.push_back(ivec2(6,7));
	_T.push_back(ivec2(7,4));
}

static void createSolidFrenetFrame(std::vector<vec3>& _V,
									std::vector<vec3>& _N,
									std::vector<vec4>& _C,
									std::vector<vec2>& _ST,
									std::vector<ivec3>& _T)
{
    _V.clear();
    _N.clear();
    _C.clear();
    _ST.clear();
    _T.clear();

    unsigned int vOffset = 0;

    for(int i = 0; i < 3; ++i)
    {
        // get local geometry
        std::vector<vec3> _cylV;
        std::vector<vec3> _cylN;
        std::vector<vec2> _cylST;
        std::vector<ivec3> _cylT;

        mat3 M[3];
        M[0] << 0, 1, 0,-1, 0, 0, 0, 0, 1;
        M[1] << 1, 0, 0, 0, 1, 0, 0, 0, 1;
        M[2] << 1, 0, 0, 0, 0,-1, 0, 1, 0;

        createSolidCylinder(vec3(0,0,0), vec3(0,1,0), REAL(0.01), 20, _cylV, _cylN, _cylST, _cylT);

        // add to output
        for(unsigned int j = 0; j < _cylV.size(); ++j)
        {
            _V.push_back(M[i]*_cylV[j]);
            //_V.push_back(vec3(i*REAL(0.3),0,0)+_cylV[j]);
            _N.push_back(_cylN[j]);
            _C.push_back(vec4((i==0),(i==1),(i==2),1));
        }

        for(unsigned int j = 0; j < _cylT.size(); ++j)
        {
            _T.push_back(ivec3(vOffset, vOffset, vOffset)+_cylT[j]);
        }

        vOffset = _V.size();
    }
}

static void createLineDots(const vec3& A, const vec3& B, std::vector<vec3>& outDots, REAL delta = 0.0001)
{
	outDots.clear();
	unsigned int numPoints = (unsigned int)((B-A).norm() / delta);

	if(numPoints < 2)
		return;

	for(unsigned int i = 0; i < numPoints; ++i)
	{
		REAL w = i * REAL(1)/REAL(numPoints-1);
		vec3 newP = (REAL(1.0)-w)*A + w*B;
		outDots.push_back(newP);
	}
}

#endif // GEOMUTILS_H
