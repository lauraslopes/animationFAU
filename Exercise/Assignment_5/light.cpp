#include "light.h"
#include "matrixutils.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>

Light::Light()
{
    mPosition = vec3(0,0,0);
    mDirection = vec3(0,0,0);
    mColor = vec4(0,0,0,0);

    // setup view matrix
    vec3 c = mPosition + mDirection;
    computeViewMatrix(mPosition, c, vec3(0,1,0));

    // setup projection
    //computeOrthoProjectionMatrix(-5,5,-5,5,-10,10, mProjMatrix);
	computePerspeciveProjectionMatrix(REAL(40),REAL(1),REAL(1.0), REAL(10.0), mProjMatrix);
}

Light::Light(const Light &other)
{
    mPosition = other.mPosition;
    mDirection = other.mDirection;
    mColor = other.mColor;

    // setup view matrix
    vec3 c = mPosition + mDirection;
    computeViewMatrix(mPosition, c, vec3(0,1,0));

    // setup projection
    //computeOrthoProjectionMatrix(-5,5,-5,5,-10,10, mProjMatrix);
	computePerspeciveProjectionMatrix(REAL(40),REAL(1),REAL(1.0), REAL(10.0), mProjMatrix);
}

Light::Light(const LightDesc &desc)
{
    mPosition = desc.position;
    mDirection = desc.direction.normalized();
    mColor = desc.color;

    // setup view matrix
    vec3 c = mPosition + mDirection;
    computeViewMatrix(mPosition, c, vec3(0,1,0));

    // setup projection
    //computeOrthoProjectionMatrix(-5,5,-5,5,-10,10, mProjMatrix);
    computePerspeciveProjectionMatrix(REAL(40),REAL(1),REAL(1.0), REAL(10.0), mProjMatrix);
}

Light::~Light()
{
    //
}

vec3 Light::getPosition() const
{
    return mPosition;
}

void Light::setPosition(const vec3 &pos)
{
    mPosition = pos;
}

vec3 Light::getDirection() const
{
    return mDirection;
}

void Light::setDirection(const vec3& dir)
{
    mDirection = dir;
}

vec4 Light::getColor() const
{
    return mColor;
}

void Light::setColor(const vec4 &col)
{
    mColor = col;
}

void Light::setUniforms(const mat4 &viewM, const int& pos_loc, const int& i_loc)
{
    // transmit
    vec4 pos_dir;
    pos_dir[0] = -mDirection[0];
    pos_dir[1] = -mDirection[1];
    pos_dir[2] = -mDirection[2];
    pos_dir[3] =  REAL(0);
    vec4 eyeLightPos = viewM * pos_dir;
    glUniform4fv(pos_loc, 1, eyeLightPos.data());
    glUniform4fv(i_loc, 1, mColor.data());
}

void Light::getViewMatrix(mat4& vm)
{
    vm = mViewMatrix;
}

void Light::computeViewMatrix(const vec3& eye,
                                const vec3& center,
                                const vec3& up)
{
    computeLookAtMatrix(eye, center, up, mViewMatrix);
}

void Light::getProjMatrix(mat4& pm)
{
    pm = mProjMatrix;
}
