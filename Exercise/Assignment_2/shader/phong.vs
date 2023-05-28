uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float shine;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;
uniform vec4 light_position;
uniform vec4 light_I;

varying vec3 N;
varying vec3 Ve;
varying vec4 C;

void main(void)
{
    vec4 posE = view_matrix * model_matrix * gl_Vertex;
    Ve = vec3(posE);
    N = normalize(normal_matrix*vec3(gl_Normal));
    C = gl_Color;
    gl_Position  = proj_matrix * posE;
}
