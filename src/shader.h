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

// Delete shader program
void shader_delete(ShaderProgram* shader);

#endif // SHADER_H
