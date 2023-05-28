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

//use this macro to convert from degrees to radiants
#define DEG_TO_RAD(x) { x*0.01745f }

// structure defining an keyframe
struct Keyframe
{
	unsigned int time;	// minimum keyframe, this animation is valid
	vec3 translation;	//
	float rotX;         // rotation about global x
	float rotY;         // rotation about global y
	float rotZ;         // rotation about global z

	Keyframe()
	{
		time = 0;
		translation = vec3::Zero();
		rotX = rotY = rotZ = 0.0f;
	}

	Keyframe(unsigned int t, const vec3& trans, float x, float y, float z)
	{
		time = t;
		translation = trans;
		rotX = x;
		rotY = y;
		rotZ = z;
	}

	Keyframe(const Keyframe& other)
	{
		time = other.time;
		translation = other.translation;
		rotX = other.rotX;
		rotY = other.rotY;
		rotZ = other.rotZ;
	}

	bool operator<(const Keyframe& rhs) { return (time < rhs.time); }
};

// structure to represent a rigid object with its
// transformation (rotation and translation)
struct RObject
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<ivec3> triangles;
	mat4 modelMatrix;

	// the animation sequence
	std::vector<Keyframe> animation;

	RObject()
	{
		vertices.clear();
		normals.clear();
		triangles.clear();
		modelMatrix = mat4::Identity();
		animation.clear();
	}

	RObject(const RObject& rhs)
	{
		vertices = rhs.vertices;
		normals = rhs.normals;
		triangles = rhs.triangles;
		modelMatrix = rhs.modelMatrix;
		animation = rhs.animation;
	}

	void interpolateTransformation(unsigned int currentFrame)
	{
		mat3 R = mat3::Identity();
		vec3 t = vec3::Zero();
		vec3 a = vec3::Zero();
		float u;
		float denominador;
		int i = 0;
		int j = 1;

		//procurando keyframes vizinhos
		if (currentFrame >= animation[animation.size() - 1].time)
			i = j = animation.size() -1;
		else{
			for (; i < animation.size(); i++){
				j = (i+1) % animation.size();
				if ((animation[i].time <= currentFrame) && (animation[j].time >= currentFrame))
					break;
			}
		}

		std::cerr << "current frame / keyframe anterior / posterior: \n" << currentFrame << i << j << std::endl;
		//calculando parte fracionária entre frames
		denominador = (float)(animation[i].time - animation[j].time);
		if (denominador != 0)
			u = (currentFrame - animation[i].time) / denominador;
		/*else*/

		//interpolando a translação
		t = ((1-u)*animation[i].translation + u*animation[j].translation);

		a[0] = ((1-u)*animation[i].rotX + u*animation[j].rotX);
		a[1] = ((1-u)*animation[i].rotY + u*animation[j].rotY);
		a[2] = ((1-u)*animation[i].rotZ + u*animation[j].rotZ);

		R = 

		// TODO - Assignment 2.2 (e): construct rotation matrix (use fixed angles approach)
		R = matriz de rotação x t x 0001 (col)
		// TODO: set the 4x4 transformation matrix
	}
};

Renderer* renderer;
ArcballCamera* camera;
RObject* roAircraft;
unsigned int frame;

void init(void)
{
	//reset global application state
	frame = 0;

	// init object emtpy
	roAircraft = new RObject();

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

	if (!renderer->addSurface("mesh", sDesc))
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

	// init animation for rigid airplane
	roAircraft->animation.push_back(Keyframe(0, vec3(0, 0, 0), 0.f, 0.f, 0));
	roAircraft->animation.push_back(Keyframe(1, vec3(3, 2, 1), 0, 0, 1));


	glutPostRedisplay();
}

void importMesh()
{
	if (roAircraft == NULL)
		return;

	// load model
	if (!importTriangleMeshFromOFF("../Media/aircraft.off", roAircraft->vertices, roAircraft->triangles))
	{
		LOG("failed to load ../Media/aircraft.off");
		exit(1);
	}
	centerMesh(roAircraft->vertices);
	computeTriangleMeshNormals(roAircraft->vertices, roAircraft->triangles, roAircraft->normals);

	Renderable *pt = renderer->getPtRenderable("mesh");
	if (!pt)
		renderer->addRenderable("mesh", roAircraft->vertices, roAircraft->triangles, "mesh", roAircraft->modelMatrix);
	else
		pt->updateVerticesAndNormals(roAircraft->vertices, roAircraft->normals);

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

	// update render mesh from rigid
	Renderable* pt = renderer->getPtRenderable("mesh");
	if (pt)
		pt->setModelMatrix(roAircraft->modelMatrix);

	// render the scene
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
	roAircraft->interpolateTransformation(frame);

	// TODO - Assignment 2.2 (a): Advance time

	// TODO - Assignment 2.2 (b): Implement a cyclic animation

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
	importMesh();

	glutMainLoop();

	return 0;
}
