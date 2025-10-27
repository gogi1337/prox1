#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

// Load shader source from file
char* shader_load_source(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open shader file '%s'\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size <= 0) {
        printf("Error: Shader file '%s' is empty or invalid\n", filename);
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        printf("Error: Failed to allocate memory for shader source\n");
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    return buffer;
}

// Compile a single shader
unsigned int shader_compile(const char* source, int shader_type) {
    unsigned int shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check compilation status
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        int log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        
        char* info_log = (char*)malloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, info_log);
        
        const char* shader_type_str = (shader_type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        printf("Error: %s shader compilation failed:\n%s\n", shader_type_str, info_log);
        
        free(info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

// Link vertex and fragment shaders into a program
ShaderProgram shader_create_program(const char* vertex_path, const char* fragment_path) {
    ShaderProgram program;
    program.program_id = 0;
    program.is_valid = false;
    
    // Load shader sources
    char* vertex_source = shader_load_source(vertex_path);
    char* fragment_source = shader_load_source(fragment_path);
    
    if (!vertex_source || !fragment_source) {
        if (vertex_source) free(vertex_source);
        if (fragment_source) free(fragment_source);
        return program;
    }
    
    // Compile shaders
    unsigned int vertex_shader = shader_compile(vertex_source, GL_VERTEX_SHADER);
    unsigned int fragment_shader = shader_compile(fragment_source, GL_FRAGMENT_SHADER);
    
    free(vertex_source);
    free(fragment_source);
    
    if (vertex_shader == 0 || fragment_shader == 0) {
        if (vertex_shader) glDeleteShader(vertex_shader);
        if (fragment_shader) glDeleteShader(fragment_shader);
        return program;
    }
    
    // Create and link program
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    
    // Check linking status
    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    
    if (!success) {
        int log_length;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);
        
        char* info_log = (char*)malloc(log_length);
        glGetProgramInfoLog(shader_program, log_length, NULL, info_log);
        
        printf("Error: Shader program linking failed:\n%s\n", info_log);
        
        free(info_log);
        glDeleteProgram(shader_program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return program;
    }
    
    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    program.program_id = shader_program;
    program.is_valid = true;
    
    printf("Shader program created successfully (ID: %u)\n", shader_program);
    return program;
}

// Use shader program
void shader_use(const ShaderProgram* shader) {
    if (shader && shader->is_valid) {
        glUseProgram(shader->program_id);
    }
}

// Set uniform values
void shader_set_int(const ShaderProgram* shader, const char* name, int value) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void shader_set_float(const ShaderProgram* shader, const char* name, float value) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void shader_set_vec2(const ShaderProgram* shader, const char* name, float x, float y) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void shader_set_vec3(const ShaderProgram* shader, const char* name, float x, float y, float z) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void shader_set_vec4(const ShaderProgram* shader, const char* name, float x, float y, float z, float w) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniform4f(location, x, y, z, w);
    }
}

void shader_set_mat4(const ShaderProgram* shader, const char* name, const float* matrix) {
    if (!shader || !shader->is_valid) return;
    int location = glGetUniformLocation(shader->program_id, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, 0, matrix);
    }
}

// Delete shader program
void shader_delete(ShaderProgram* shader) {
    if (shader && shader->is_valid) {
        glDeleteProgram(shader->program_id);
        shader->program_id = 0;
        shader->is_valid = false;
    }
}