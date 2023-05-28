#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "platform.h"
#include "geomutils.h"

// import OFF
static bool importTriangleMeshFromOFF(const std::string& filename, std::vector<vec3>& vertices, std::vector<ivec3>& triangles)
{
	vertices.clear();
    triangles.clear();

    std::ifstream inF(filename.c_str());

    if(!inF.good())
    {
        PRINTERROR("importTriangleMeshFromOFF error: file " << filename.c_str() << " not found");
        return false;
    }

    char head[5];
    inF >> head;
    int numV = 0, numT = 0, numE = 0;

    inF >> numV >> numT >> numE;
    for(int i = 0; i < numV; ++i)
    {
        vec3 mI;
        inF >> mI[0] >> mI[1] >> mI[2];
        vertices.push_back(mI);
    }

    for(int i2 = 0; i2 < numT; ++i2)
    {
        int numF;
        ivec3 tI;
        inF >> numF >> tI[0] >> tI[1] >> tI[2];
        triangles.push_back(tI);
    }

    return true;
}

// import COFF
static bool importTriangleMeshFromCOFF(const std::string& filename, std::vector<vec3>& _V, std::vector<vec4>& _C, std::vector<ivec3>& _T)
{
    _V.clear();
    _C.clear();
    _T.clear();

    std::ifstream inF(filename.c_str());

    if(!inF.good())
    {
        PRINTERROR("importTriangleMeshFromOFF error: file " << filename.c_str() << " not found");
        return false;
    }

    char head[5];
    inF >> head;
    int numV = 0, numT = 0, numE = 0;

    inF >> numV >> numT >> numE;
    for(int i = 0; i < numV; ++i)
    {
        vec3 mI;
        vec4 cI;
        inF >> mI[0] >> mI[1] >> mI[2] >> cI[0] >> cI[1] >> cI[2] >> cI[3];
        _V.push_back(mI);
        _C.push_back(cI);
    }

    for(int i = 0; i < numT; ++i)
    {
		unsigned int d;
        ivec3 t;
        inF >> d >> t[0] >> t[1] >> t[2];
        _T.push_back(t);
    }

    return true;
}

// export OFF
static bool exportToOFF(const std::string &filename, const std::vector<vec3> &vertices, const std::vector<ivec3> &triangles)
{
    std::ofstream of(filename.c_str());

    if(of.fail())
    {
        PRINTERROR("exportToOff error: failed to open file " << filename);
        return false;
    }

    of << "OFF\n";
    of << vertices.size() << " " << triangles.size() << " 0\n";

    for(unsigned int i = 0; i < vertices.size(); ++i)
    {
        of << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << "\n";
    }

    for(unsigned int j = 0; j < triangles.size(); ++j)
    {
        if((triangles[j][0] >= vertices.size()) || (triangles[j][1] >= vertices.size()) || (triangles[j][2] >= vertices.size()))
        {
            PRINTERROR("exportToOff error: face index out of vertex bounds");
            of.close();
            return false;
        }

        of << "3 " << triangles[j][0] << " " << triangles[j][1] << " " << triangles[j][2] << "\n";
    }

    of.close();

    return true;
}

// export COFF
static bool exportToColoredOff(const std::string &filename,
                                    const std::vector<vec3> &vertices,
									const std::vector<vec4> &colors,
									const std::vector<ivec3> &triangles)
{
    std::ofstream of(filename.c_str());

    if(of.fail())
    {
        PRINTERROR("exportToColoredOff error: failed to open file " << filename);
        return false;
    }

    of << "COFF\n";
    of << vertices.size() << " " << triangles.size() << " 0\n";

    if(vertices.size() != colors.size())
    {
        PRINTERROR("exportToColoredOff error: size of color does not match size of vertices");
        return false;
    }

    for(unsigned int i = 0; i < vertices.size(); ++i)
    {
        of << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << " ";
        of << (int)colors[i][0] << " " << (int)colors[i][1] << " " << (int)colors[i][2] << " " << (int)colors[i][3] << "\n";
    }

    for(unsigned int j = 0; j < triangles.size(); ++j)
    {
        if((triangles[j][0] >= vertices.size()) || (triangles[j][1] >= vertices.size()) || (triangles[j][2] >= vertices.size()))
        {
            PRINTERROR("exportToColoredOff error: face index out of vertex bounds");
            of.close();
            return false;
        }

        of << "3 " << triangles[j][0] << " " << triangles[j][1] << " " << triangles[j][2] << "\n";
    }

    of.close();

    return true;
}

#endif // FILEUTILS_H
