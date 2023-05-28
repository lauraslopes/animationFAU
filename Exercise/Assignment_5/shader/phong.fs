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
    vec4 I_d = vec4(0);
    vec4 I_s = vec4(0);
    vec3 E = normalize(-Ve);
    vec3 L = normalize(vec3(light_position));
    float lambert = max(dot(N,L), 0.0);
    
    if(lambert > 0.0)
    {
        I_d = C * light_I * diffuse * lambert;
        
        vec3 R = normalize(-reflect(L,N));
        I_s = C * light_I * specular * pow(max(dot(R,E), 0.1), shine);
    }

    gl_FragColor = I_d + I_s;
    gl_FragColor = max(gl_FragColor, C * ambient);
}
