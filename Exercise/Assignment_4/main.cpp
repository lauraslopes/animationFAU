#include <GL/freeglut.h>
#include "fileutils.h"
#include "geomutils.h"
#include "matrixutils.h"
#include "renderer.h"
#include "renderable.h"
#include "surface.h"
#include "camera.h"
#include "light.h"

#define WIDTH 1024
#define HEIGHT 768
#define NUM_SAMPLES 4
#define POINT_RADIUS 0.003

#define GRID_X 10
#define GRID_Y 10

struct Spring
{
	unsigned int p0;
	unsigned int p1;
	float restLength;
	float D;

	Spring()
	{
		p0 = 0;
		p1 = 0;
		restLength = 0.0f;
		D = 1.0f;
	}

	Spring(const Spring& other)
	{
		p0 = other.p0;
		p1 = other.p1;
		restLength = other.restLength;
		D = other.D;
	}
};

struct Mesh 
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	std::vector<vec3> positions;
	std::vector<vec3> velocities;
	std::vector<vec3> forces;
	std::vector<float> invMass;
	
	std::vector<vec3> normals;
	std::vector<ivec3> triangles;

	// topology
	std::vector<Spring> springs;

	Mesh()
	{
		positions.clear();
		velocities.clear();
		forces.clear();
		normals.clear();
		invMass.clear();
		triangles.clear();
		springs.clear();
	}
};

Renderer* renderer;
ArcballCamera* camera;
Mesh* mesh;
bool running = false;

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
	
	glPolygonMode(GL_FRONT, GL_LINE);

	// load a surface material
	SurfaceDesc sDesc;
	sDesc.shaderType = SurfaceDesc::SHADER_PHONG;
	sDesc.diffuse_map = "";
	sDesc.normal_map = "";
	sDesc.ambient = vec4(REAL(0.0), REAL(0.0), REAL(0.0), REAL(1));
	sDesc.diffuse = vec4(REAL(1.0), REAL(0.0), REAL(0.0), REAL(1));
	sDesc.specular = vec4(REAL(0.0), REAL(0.0), REAL(0.0), REAL(1));
	sDesc.shine = REAL(32);
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

static unsigned int flatIndex(unsigned int i, unsigned int j)
{
	// TODO
	return 0;
}

void initMesh()
{
	SAFE_DELETE(mesh);
	mesh = new Mesh;

	unsigned int width = GRID_X;
	unsigned int height = GRID_Y;
	float delta = 0.1f;

	// TODO: create vertices in mesh->positions

	// TODO: create triangles in mesh->triangles
	
	// TODO: init renderer
	
	// TODO: init normals, velocities, forces, and inverse masses (1/mass)
	
	// TODO: fix some vertices by setting mass to infinity
	
	// TODO: create springs
	
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
	if (!running)
		return;
	
	// simulate 
	float dT = 0.001f;
	
	// TODO: cumulate forces
	

	// TODO: integrate particle positions
	
	// TODO: update renderer
	
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

	case 's':
		// deform some bones
		running = !running;
		break;

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
	initMesh();
	
	glutMainLoop();
	
	return 0;
}
