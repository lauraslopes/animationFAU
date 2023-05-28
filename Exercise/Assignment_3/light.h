#ifndef LIGHT_H
#define LIGHT_H

#include "platform.h"

struct LightDesc
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    vec3 position;
    vec3 direction;
    vec4 color;

    LightDesc()
    {
        position = vec3(0,0,0);
        direction = vec3(0,0,0);
        color = vec4(0,0,0,0);
    }
};

class Light
{

public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    //! standard constructor (white pointlight in origin)
    Light();

    //! copy constructor
    Light(const Light& other);

    //! initialize light with a pos (w == 1 point, w == 0 directional)
    Light(const LightDesc& desc);

    //! destructor
    ~Light();

    //! get the position
    vec3 getPosition() const;

    //! set the position
    void setPosition(const vec3& pos);

    //! get the direction
    vec3 getDirection() const;

    //! set direction
    void setDirection(const vec3& dir);

    //! get the color
    vec4 getColor() const;

    //! set the color
    void setColor(const vec4& col);

    //! set the light to the given shader
    void setUniforms(const mat4& viewM, const int& pos_loc, const int& i_loc);

    //! compute a look at matrix for given eye, focus, up
    void computeViewMatrix(const vec3& eye,
                            const vec3& center,
                            const vec3& up);

    //! getter/setter for the view matrix
    void getViewMatrix(mat4 &vm);

    //! getter/setter for the proj matrix
    void getProjMatrix(mat4 &pm);

protected:

    //! lightsource's position
    vec3 mPosition;

    //! lightsource's direction
    vec3 mDirection;

    //! emitted color
    vec4 mColor;

    //! light sources view matrix
    mat4 mViewMatrix;

    //! light sources projection matrix
    mat4 mProjMatrix;
};

#endif // LIGHT_H
