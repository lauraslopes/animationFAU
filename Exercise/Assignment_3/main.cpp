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

//use this macro to convert from degrees to radiants
#define DEG_TO_RAD(x) { x*0.01745f }

// structure representing an axis aligned bounding box
struct AABB
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	vec3 minPosition;
	vec3 maxPosition;

	AABB()
	{
		minPosition = maxPosition = vec3::Zero();
	}

	void setFromVertices(const std::vector<vec3>& vertices)
	{
		//TODO: expand the Axis Aligned Bounding Box from the vertices
	}
};

// static helper to transform a vec3 by a mat4
static void transform(const mat4& M, const vec3& vIn, vec3& vOut)
{
	vec4 v4;
	v4 << vIn, 1;
	vec4 r = M*v4;
	vOut = vec3(r.x()/r.w(), r.y()/r.w(), r.z()/r.w());
}

// structure to represent a rigid object with its
// transformation (rotation and translation)
struct RObject
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<ivec3> triangles;
	mat4 modelMatrix;
	vec3 velocity;
	AABB aabb;
	Renderable* ptRenderable;

	RObject()
	{
		vertices.clear();
		normals.clear();
		triangles.clear();
		modelMatrix = mat4::Identity();
		velocity = vec3::Zero();
		ptRenderable = NULL;
	}

	void setTransformation(const mat3& R, const vec3& t)
	{
		modelMatrix << R, t, 0, 0, 0, 1;
	}

	void setVelocity(const vec3& v)
	{
		velocity = v;
	}

	void move()
	{
		addTranslation(velocity);
	}

	void addTranslation(const vec3& t)
	{
		modelMatrix(0, 3) += t[0];
		modelMatrix(1, 3) += t[1];
		modelMatrix(2, 3) += t[2];
	}

	bool collision(const RObject& other)
	{
		// TODO: check for collision of this object with the given parameter object
		// HINT: do not forget that AABB is stored in local object space

		return false;
	}
};

// globals
Renderer* renderer;
ArcballCamera* camera;
std::vector<RObject*> rigids;
bool running;

void init(void)
{
	running = false;

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
	sDesc.ambient = vec4(0.1f, 0.1f, 0.1f, 1.0f);
	sDesc.diffuse = vec4(0.6f, 0.6f, 0.6f, 1.0f);
	sDesc.specular = vec4(0.4f, 0.2f, 0.3f, 1.0f);
	sDesc.shine = 2.0f;

	if (!renderer->addSurface("material0", sDesc))
	{
		LOG("add material0 failed");
		SAFE_DELETE(camera);
		exit(1);
	}

	sDesc.diffuse = vec4(0.5f, 0.4f, 0.3f, 1.0f);
	if (!renderer->addSurface("material1", sDesc))
	{
		LOG("add material1 failed");
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

void initModels()
{
	rigids.resize(3, NULL);

	// init model 0
	//
	rigids[0] = new RObject;
	rigids[0]->setTransformation(mat3::Identity(), vec3(-1, 0, 0));
	rigids[0]->setVelocity(vec3(0, 0, 0));
	importTriangleMeshFromOFF("../Media/bunny.off", rigids[0]->vertices, rigids[0]->triangles);
	centerMesh(rigids[0]->vertices);
	rigids[0]->aabb.setFromVertices(rigids[0]->vertices);
	computeTriangleMeshNormals(rigids[0]->vertices, rigids[0]->triangles, rigids[0]->normals);
	renderer->addRenderable("mesh0", rigids[0]->vertices, rigids[0]->triangles, "material0", rigids[0]->modelMatrix);
	rigids[0]->ptRenderable = renderer->getPtRenderable("mesh0");

	// init model 1
	//
	rigids[1] = new RObject;
	rigids[1]->setTransformation(mat3::Identity(), vec3(0.5f, 0, 0));
	rigids[1]->setVelocity(vec3(-0.01f, 0, 0));
	importTriangleMeshFromOFF("../Media/sphere.off", rigids[1]->vertices, rigids[1]->triangles);
	centerMesh(rigids[1]->vertices);
	rigids[1]->aabb.setFromVertices(rigids[1]->vertices);
	computeTriangleMeshNormals(rigids[1]->vertices, rigids[1]->triangles, rigids[1]->normals);
	renderer->addRenderable("mesh1", rigids[1]->vertices, rigids[1]->triangles, "material1", rigids[1]->modelMatrix);
	rigids[1]->ptRenderable = renderer->getPtRenderable("mesh1");

	// init model 2
	//
	rigids[2] = new RObject;
	rigids[2]->setTransformation(mat3::Identity(), vec3(1, 0, 0));
	rigids[2]->setVelocity(vec3(0, 0, 0));
	importTriangleMeshFromOFF("../Media/bunny.off", rigids[2]->vertices, rigids[2]->triangles);
	centerMesh(rigids[2]->vertices);
	rigids[2]->aabb.setFromVertices(rigids[2]->vertices);
	computeTriangleMeshNormals(rigids[2]->vertices, rigids[2]->triangles, rigids[2]->normals);
	renderer->addRenderable("mesh2", rigids[2]->vertices, rigids[2]->triangles, "material0", rigids[2]->modelMatrix);
	rigids[2]->ptRenderable = renderer->getPtRenderable("mesh2");
	
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

	// render models
	for (unsigned int i = 0; i < rigids.size(); ++i)
	{
		// update render mesh from rigid
		std::ostringstream oss;
		oss << "mesh" << i;
		Renderable* pt = renderer->getPtRenderable(oss.str());
		if (pt) pt->setModelMatrix(rigids[i]->modelMatrix);
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
	if (!running)
		return;

	// move objects
	
	// TODO: move the objects
	
	// check for collisions
	// TODO: check for each object-object collision and react on detected collisions

	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
	// catch the wheel event
	if (button == 3)
	{
		camera->addRadius(0.01f);
		return;
	}

	if (button == 4)
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
	std::cerr << "wheel " << dir / 30.0f;

	glutPostRedisplay();
}

void key(unsigned char key, int x, int y)
{
	switch (key) {

	case 27: // ESCAPE KEY
		shutdown();
		exit(0);
		break;

	case 'r': // r-key (fall down to space key) start stop animation
	case ' ': // space key
		// start stop animation
		running = !running;
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
	initModels();

	glutMainLoop();

	return 0;
}
