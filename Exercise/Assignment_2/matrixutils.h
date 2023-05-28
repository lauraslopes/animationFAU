#ifndef MATRIXUTILS_H
#define MATRIXUTILS_H

#include "platform.h"

static void computePerspeciveProjectionMatrix(const REAL& fovy,
                                                const REAL& aspect,
                                                const REAL& near_plane,
                                                const REAL& far_plane,
                                                mat4& outM)
{
    REAL fovy_rad = fovy * REAL(M_PI) / 360.0f;
    REAL y_scale = cosf(fovy_rad) / sinf(fovy_rad);
    REAL x_scale = y_scale / aspect;
    REAL c_23 = -((far_plane+near_plane) / (far_plane-near_plane));
    REAL c_33 = -((2.0f*near_plane*far_plane) / (far_plane-near_plane));

    outM << x_scale, 0, 0, 0,
            0, y_scale, 0, 0,
            0, 0, c_23, c_33,
            0, 0, -1, 0;
}

static void computeOrthoProjectionMatrix(const REAL& left,
                                            const REAL& right,
                                            const REAL& bottom,
                                            const REAL& top,
                                            const REAL& near_plane,
                                            const REAL& far_plane,
                                            mat4& out)
{
    REAL x_scale = REAL(2) / (right - left);
    REAL y_scale = REAL(2) / (top - bottom);
    REAL z_scale = -REAL(2) / (far_plane - near_plane);
    REAL x_offset = -(right+left)/(right-left);
    REAL y_offset = -(top+bottom)/(top-bottom);
    REAL z_offset = -(far_plane+near_plane)/(far_plane-near_plane);

    out << x_scale, 0, 0, x_offset,
            0, y_scale, 0, y_offset,
            0, 0, z_scale, z_offset,
            0, 0, 0, 1;
}

static void computeLookAtMatrix(const vec3& eye, const vec3& center, const vec3& up, mat4& outM)
{
    vec3 z = (eye - center).normalized();
    vec3 x = (up.cross(z)).normalized();
    vec3 y = z.cross(x);

    mat4 rotM;
    rotM << x.x(), x.y(), x.z(), 0,
            y.x(), y.y(), y.z(), 0,
            z.x(), z.y(), z.z(), 0,
            0,0,0,1;

    mat4 trfM;
    trfM << 1, 0, 0, -eye.x(),
            0, 1, 0, -eye.y(),
            0, 0, 1, -eye.z(),
            0, 0, 0, 1;

    outM = rotM * trfM;
}

static void extractEyePosFromViewMatrix(const mat4& viewM, vec3& out)
{
    mat4 Rt = viewM.transpose();
    Rt(3,0) = Rt(3,1) = Rt(3,2) = REAL(0);
    mat4 D = Rt * viewM;
    out = -D.block<3,1>(0,3);
}

#endif // MATRIXUTILS_H
