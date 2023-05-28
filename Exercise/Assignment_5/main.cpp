#include <GL/freeglut.h>
#include "fileutils.h"
#include "geomutils.h"
#include "matrixutils.h"
#include "renderer.h"
#include "renderable.h"
#include "surface.h"
#include "camera.h"
#include "light.h"
#include "skeleton.h"

#define WIDTH 1024
#define HEIGHT 768
#define NUM_SAMPLES 4
#define POINT_RADIUS 0.003

// attachment of a single vertex to all affected bones
struct Attachment
{
	std::vector<unsigned int> boneIds;
	std::vector<float> weights;
	std::vector<vec3> localPositions;

	Attachment()
	{
		boneIds.clear();
		weights.clear();
		localPositions.clear();
	}

	Attachment(const Attachment& other)
	{
		boneIds = other.boneIds;
		weights = other.weights;
		localPositions = other.localPositions;
	}
};

struct Mesh 
{
	std::vector<vec3> verticesInLoadPose;
	std::vector<vec3> normalsInLoadPose;
	std::vector<Attachment> attachments;
	std::vector<ivec3> triangles;
	MakeHSkeleton skeleton;
	bool dirty;

	Mesh()
	{
		dirty = false;
		verticesInLoadPose.clear();
		normalsInLoadPose.clear();
		triangles.clear();
	}

	void rotateBone(unsigned int boneId, const vec3& angles)
	{
		if (boneId >= skeleton.getNumBones())
			return;

        // TODO: implement a relative bone rotation using Skeleton::setBoneRotationAngles

		dirty = true;
	}
};

Renderer* renderer;
ArcballCamera* camera;
Mesh* mesh;

void init(void)
{
	mesh = NULL;

	// init camera
	camera = new ArcballCamera();
	camera->setLookAt(vec3(0, 0, 3), vec3(0, 0, 0), vec3(0, 1, 0));

	// init renderer
	renderer = new Renderer();
	if (!renderer->init(WIDTH, HEIGHT, "./shader/", 1))
	{
		LOG("gl initialization failed");
		SAFE_DELETE(camera);
		exit(1);
	}
	renderer->setClearColor(vec4(0.1f, 0.1f, 0.2f, 1.0f));
	
	// load a surface material
	SurfaceDesc sDesc;
	sDesc.shaderType = SurfaceDesc::SHADER_PHONG;
	sDesc.diffuse_map = "";
	sDesc.normal_map = "";
	sDesc.ambient = vec4(REAL(0.3), REAL(0.1), REAL(0.1), REAL(1));
	sDesc.diffuse = vec4(REAL(0.96), REAL(0.76), REAL(0.76), REAL(1));
	sDesc.specular = vec4(REAL(0.04), REAL(0.02), REAL(0.02), REAL(1));
	sDesc.shine = REAL(2);
	if (!renderer->addSurface("meshMaterial", sDesc))
	{
		LOG("add surface failed");
		SAFE_DELETE(camera);
		exit(1);
	}
	
	// init miner's light
	LightDesc lDesc;
	lDesc.color = vec4(1, 1, 1, 1);
	lDesc.position = vec3(0, 5, 0);
	lDesc.direction = vec3(0, -1, 0);
	renderer->addLight("light1", lDesc);

	glutPostRedisplay();
}

void loadMeshAndRig()
{
	SAFE_DELETE(mesh);
	mesh = new Mesh;

	// load mesh
	importTriangleMeshFromOFF("../Media/avatar.off", mesh->verticesInLoadPose, mesh->triangles);
	centerMesh(mesh->verticesInLoadPose);
	computeTriangleMeshNormals(mesh->verticesInLoadPose, mesh->triangles, mesh->normalsInLoadPose);
	renderer->addRenderable("mesh", mesh->verticesInLoadPose, mesh->triangles, "meshMaterial", mat4::Identity());

	// init skeleton
	mesh->skeleton.fitToMakeHMesh(mesh->verticesInLoadPose);

	// load attachment file
	std::ifstream attFile("../Media/avatarAtt.txt");
    
    // TODO: load attachment for each vertex


    attFile.close();


	mesh->dirty = true;

	glutPostRedisplay();
}

void shutdown(void)
{
	SAFE_DELETE(camera);
	SAFE_DELETE(renderer);
}

void display(void)
{
	// setup light to be a miners lamp
	mat4 lM;
	vec3 lPos, lDir;
	camera->getViewMatrix(lM);
	extractEyePosFromViewMatrix(lM, lPos);
	lM.transposeInPlace();
	lDir = -lM.block<3, 1>(0, 2).normalized();
	renderer->getPtLight("light1")->setPosition(lPos);
	renderer->getPtLight("light1")->setDirection(lDir);
	
	// check if the skeleton has changed and change rendering!!!
	if (mesh->dirty)
	{
        // TODO: 
		mesh->dirty = false;
	}
		
	renderer->render((Camera*)camera);
	glutSwapBuffers();
}

void resize(int w, int h)
{
	REAL aspect = REAL(w) / REAL((h == 0) ? 1 : h);
	renderer->resize(w, h);
	camera->resize(w, h);
	camera->setPerspectiveProjection(REAL(45), aspect, REAL(0.1), REAL(100.0));
	glutPostRedisplay();
}

void idle()
{
	//

	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
    // catch the wheel event
    if(button == 3)
    {
        camera->addRadius(0.01f);
        return;
    }
    
    if(button == 4)
    {
        camera->addRadius(-0.01f);
        return;
    }

	switch (state)
	{
	case GLUT_DOWN:
		if (button == GLUT_RIGHT_BUTTON)
		{
			camera->startMovement(x, y);
		}
		break;

	case GLUT_UP:
		if (button == GLUT_RIGHT_BUTTON)
		{
			// deactivate camera movement
			camera->stopMovement();
		}
		break;

	default:
		break;
	}
}

void mouseMove(int x, int y)
{
	camera->move(x, y);

	glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
    std::cerr << "wheel " << dir/30.0f;

	glutPostRedisplay();
}

void key(unsigned char key, int x, int y)
{
	switch (key) {

	case 27: // ESCAPE KEY
		shutdown();
		exit(0);
		break;
    
    // TODO: set some bone rotations by key press

	default:
		break;
	}

	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	// init window and gl
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Animation Framework");
		
	// register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);
	glutKeyboardFunc(key);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutCloseFunc(shutdown);

	init();
	loadMeshAndRig();
	
	glutMainLoop();
	
	return 0;
}
