#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>

// Vertex data structure for GPU
typedef struct {
    float position[2];
    float color[4];
} ParticleVertex;

// Helper function to check OpenGL errors (debug only)
static void check_gl_error(const char* operation) {
#ifdef DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL Error after %s: 0x%x\n", operation, error);
    }
#endif
}

Renderer* renderer_create() {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (!renderer) {
        fprintf(stderr, "Error: Failed to allocate renderer\n");
        return NULL;
    }
    
    renderer->vao = 0;
    renderer->vbo = 0;
    renderer->fade_vao = 0;
    renderer->fade_vbo = 0;
    renderer->particle_shader.program_id = 0;
    renderer->particle_shader.is_valid = false;
    renderer->fade_shader.program_id = 0;
    renderer->fade_shader.is_valid = false;
    renderer->initialized = false;
    renderer->particle_count = 0;
    
    return renderer;
}

bool renderer_init(Renderer* renderer, int window_width, int window_height) {
    if (!renderer) return false;
    
    // Create particle VAO/VBO
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location = 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 
                          (void*)offsetof(ParticleVertex, color));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    check_gl_error("Particle VAO setup");
    
    // Create fade quad VAO/VBO (full-screen quad in NDC)
    float fade_vertices[] = {
        -1.0f, -1.0f,  // Bottom-left
         1.0f, -1.0f,  // Bottom-right
         1.0f,  1.0f,  // Top-right
        -1.0f,  1.0f   // Top-left
    };
    
    glGenVertexArrays(1, &renderer->fade_vao);
    glGenBuffers(1, &renderer->fade_vbo);
    
    glBindVertexArray(renderer->fade_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->fade_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fade_vertices), fade_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    check_gl_error("Fade VAO setup");
    
    // Load particle shader
    renderer->particle_shader = shader_create_program("shaders/particle.vert", "shaders/particle.frag");
    
    if (!renderer->particle_shader.is_valid) {
        fprintf(stderr, "Error: Failed to create particle shader program\n");
        return false;
    }
    
    // Load fade shader
    renderer->fade_shader = shader_create_program("shaders/fade.vert", "shaders/fade.frag");
    
    if (!renderer->fade_shader.is_valid) {
        fprintf(stderr, "Error: Failed to create fade shader program\n");
        return false;
    }
    
    // Get uniform locations
    renderer->particle_projection_loc = glGetUniformLocation(renderer->particle_shader.program_id, "u_projection");
    renderer->fade_color_loc = glGetUniformLocation(renderer->fade_shader.program_id, "u_fade_color");
    
    check_gl_error("Shader uniform locations");
    
    // Set OpenGL state (once)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Initial clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    renderer->initialized = true;
    renderer_set_viewport(renderer, window_width, window_height);
    
    printf("Renderer initialized successfully\n");
    printf("Particle shader ID: %d\n", renderer->particle_shader.program_id);
    printf("Fade shader ID: %d\n", renderer->fade_shader.program_id);
    
    return true;
}

void renderer_update_particles(Renderer* renderer, const ParticleSystem* ps) {
    if (!renderer || !renderer->initialized || !ps || ps->count == 0) return;
    
    // Allocate vertex data (2 vertices per particle for line rendering)
    ParticleVertex* vertices = (ParticleVertex*)malloc(sizeof(ParticleVertex) * ps->count * 2);
    if (!vertices) {
        fprintf(stderr, "Error: Failed to allocate particle vertices\n");
        return;
    }
    
    // Build vertex data (lines from prev_position to position)
    for (int i = 0; i < ps->count; i++) {
        const Particle* p = &ps->particles[i];
        int idx = i * 2;
        
        // Start vertex (previous position)
        vertices[idx].position[0] = p->prev_position.x;
        vertices[idx].position[1] = p->prev_position.y;
        vertices[idx].color[0] = p->color[0];
        vertices[idx].color[1] = p->color[1];
        vertices[idx].color[2] = p->color[2];
        vertices[idx].color[3] = p->color[3] * 0.5f;
        
        // End vertex (current position)
        vertices[idx + 1].position[0] = p->position.x;
        vertices[idx + 1].position[1] = p->position.y;
        vertices[idx + 1].color[0] = p->color[0];
        vertices[idx + 1].color[1] = p->color[1];
        vertices[idx + 1].color[2] = p->color[2];
        vertices[idx + 1].color[3] = p->color[3];
    }
    
    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertex) * ps->count * 2, 
                 vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    renderer->particle_count = ps->count * 2;
    
    free(vertices);
    check_gl_error("Update particles");
}

void renderer_request_clear(Renderer* renderer) {
    if (renderer) {
        renderer->should_clear = true;
    }
}

void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Config* config, const Camera* cam) {
    if (!renderer || !renderer->initialized || !ps || renderer->particle_count == 0) return;
    
    // Handle clear request
    if (renderer->should_clear) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer->should_clear = false;
    } else {
        glUseProgram(renderer->fade_shader.program_id);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glBindVertexArray(renderer->fade_vao);
        
        float fade_color[4] = {0.0f, 0.0f, 0.0f, 0.08f};  // Was 0.015f
        glUniform4fv(renderer->fade_color_loc, 1, fade_color);
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        check_gl_error("Draw fade");
    }
    
    // Get camera view bounds
    float left, right, bottom, top;
    camera_get_view_bounds(cam, &left, &right, &bottom, &top);
    
    // Create orthographic projection matrix
    float projection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f
    };
    
    glUseProgram(renderer->particle_shader.program_id);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glUniformMatrix4fv(renderer->particle_projection_loc, 1, GL_FALSE, projection);
    
    glBindVertexArray(renderer->vao);
    glLineWidth(1.5f);
    
    // Draw all particle trails as lines
    glDrawArrays(GL_LINES, 0, renderer->particle_count);
    
    check_gl_error("Draw particles");
    
    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderer_set_viewport(Renderer* renderer, int width, int height) {
    if (!renderer) return;
    glViewport(0, 0, width, height);
}

void renderer_destroy(Renderer* renderer) {
    if (renderer) {
        if (renderer->initialized) {
            glDeleteVertexArrays(1, &renderer->vao);
            glDeleteBuffers(1, &renderer->vbo);
            glDeleteVertexArrays(1, &renderer->fade_vao);
            glDeleteBuffers(1, &renderer->fade_vbo);
            shader_delete(&renderer->particle_shader);
            shader_delete(&renderer->fade_shader);
        }
        free(renderer);
    }
}