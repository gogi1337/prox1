#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>

// Shader program structure
typedef struct {
    unsigned int program_id;
    bool is_valid;
} ShaderProgram;

// Load shader source from file
char* shader_load_source(const char* filename);

// Compile a single shader
unsigned int shader_compile(const char* source, int shader_type);

// Link vertex and fragment shaders into a program
ShaderProgram shader_create_program(const char* vertex_path, const char* fragment_path);

// Use shader program
void shader_use(const ShaderProgram* shader);

// Set uniform values
void shader_set_int(const ShaderProgram* shader, const char* name, int value);
void shader_set_float(const ShaderProgram* shader, const char* name, float value);
void shader_set_vec2(const ShaderProgram* shader, const char* name, float x, float y);
void shader_set_vec3(const ShaderProgram* shader, const char* name, float x, float y, float z);
void shader_set_vec4(const ShaderProgram* shader, const char* name, float x, float y, float z, float w);
void shader_set_mat4(const ShaderProgram* shader, const char* name, const float* matrix);

// Delete shader program
void shader_delete(ShaderProgram* shader);

#endif // SHADER_H
