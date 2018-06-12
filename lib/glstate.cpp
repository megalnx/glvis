#include "glstate.hpp"
#include <string>

std::string vertex_shader_file = 
"#version 120 \
 \
//attribute vec4 gl_Vertex; \
//attribute vec4 gl_Color; \
//attribute vec4 gl_Normal; \
//attribute vec4 gl_MultiTexCoord0; \
 \
uniform mat4 modelViewMatrix; \
uniform mat4 projectionMatrix; \
uniform mat3 normalMatrix; \
 \
varying vec3 fNormal; \
varying vec3 fPosition; \
varying vec4 fColor; \
varying vec2 fTexCoord; \
varying vec2 fFontTexCoord; \
 \
void main() \
{ \
    fNormal = normalize(normalMatrix * gl_Normal); \
    vec4 pos = modelViewMatrix * gl_Vertex; \
    fPosition = pos.xyz; \
    fColor = gl_Color; \
    fTexCoord = gl_MultiTexCoord0.xy; \
    fFontTexCoord = gl_MultiTexCoord1.xy; \
    gl_Position = projectionMatrix * pos; \
}";

std::string fragment_shader_file =
"#version 120 \
 \
uniform bool containsText; \
uniform bool useColorTex; \
 \
uniform sampler2D fontTex; \
uniform sampler2D colorTex; \
 \
varying vec3 fNormal; \
varying vec3 fPosition; \
varying vec4 fColor; \
varying vec2 fTexCoord; \
varying vec2 fFontTexCoord; \
 \
struct PointLight { \
    vec3 position; \
    vec4 diffuse; \
    vec4 specular; \
}; \
 \
uniform int numLights; \
uniform PointLight lights[3]; \
uniform vec4 g_ambient; \
 \
struct Material { \
    vec4 ambient; \
    vec4 diffuse; \
    vec4 specular; \
    float shininess; \
}; \
 \
uniform Material material; \
 \
void main() \
{ \
    if (containsText) { \
        gl_FragColor = texture2D(fontTex, fFontTexCoord); \
    } else { \
        vec4 color = fColor; \
        if (useColorTex) { \
            color = texture2D(colorTex, fTexCoord); \
        } \
        vec4 ambient_light = g_ambient * material.ambient; \
        vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 0.0); \
        vec4 specular_light = vec4(0.0, 0.0, 0.0, 0.0); \
        for (int i = 0; i < numLights; i++) { \
            vec3 light_dir = normalize(lights[i].position - fPosition); \
            diffuse_light += lights[i].diffuse * material.diffuse * max(dot(fNormal, light_dir), 0.0); \
 \
            vec3 eye_to_vert = normalize(-fPosition); \
            vec3 vert_to_light = reflect(-light_dir, fNormal); \
            float specular_factor = max(dot(eye_to_vert, vert_to_light), 0.0); \
            specular_light += lights[i].specular * material.specular * pow(specular_factor, material.shininess); \
        } \
        gl_FragColor = color * (ambient_light + diffuse_light + specular_light); \
    } \
}";

bool GlState::compileShaders() {
    GLuint vtx_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    GLint success = 0;
    int vertex_shader_len = vertex_shader_file.length();
    glShaderSource(vtx_shader, 1, vertex_shader_file.c_str(), &vertex_shader_len);
    glCompileShader(vtx_shader);
    glGetShaderiv(vtx_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "FATAL: Vertex shader compilation failed." << endl;
        return false;
    }
    int frag_shader_len = fragment_shader_file.length();
    glShaderSource(frag_shader, 1, fragment_shader_file.c_str(), &frag_shader_len);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "FATAL: Fragment shader compilation failed." << endl;
        return false;
    }
    program = glCreateProgram();
    glAttachShader(program, vtx_shader);
    glAttachShader(program, frag_shader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        cerr << "FATAL: Shader linking failed." << endl;
        glDeleteProgram(program);
        program = 0;
    } else {
        glUseProgram(program);
    }

    glDetachShader(program, vtx_shader);
    glDetachShader(program, frag_shader);
    glDeleteShader(vtx_shader);
    glDeleteShader(frag_shader);
    return (success != GL_FALSE);
}
