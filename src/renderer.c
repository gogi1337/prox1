#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <GL/gl.h>

// Vertex data structure for GPU
typedef struct {
    float position[2];
    float color[4];
} ParticleVertex;

Renderer* renderer_create() {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (!renderer) {
        printf("Error: Failed to allocate renderer\n");
        return NULL;
    }
    
    renderer->vao = 0;
    renderer->vbo = 0;
    renderer->shader.program_id = 0;
    renderer->shader.is_valid = false;
    renderer->initialized = false;
    
    return renderer;
}

bool renderer_init(Renderer* renderer, int window_width, int window_height) {
    if (!renderer) return false;
    
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 
                          (void*)offsetof(ParticleVertex, color));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    renderer->shader = shader_create_program("shaders/particle.vert", "shaders/particle.frag");
    
    if (!renderer->shader.is_valid) {
        printf("Error: Failed to create shader program\n");
        return false;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Clear once at start
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    renderer->initialized = true;
    renderer_set_viewport(renderer, window_width, window_height);
    
    printf("Renderer initialized successfully\n");
    return true;
}

void renderer_update_particles(Renderer* renderer, const ParticleSystem* ps) {
    if (!renderer || !renderer->initialized || !ps) return;
    
    ParticleVertex* vertices = (ParticleVertex*)malloc(sizeof(ParticleVertex) * ps->count);
    if (!vertices) return;
    
    for (int i = 0; i < ps->count; i++) {
        vertices[i].position[0] = ps->particles[i].position.x;
        vertices[i].position[1] = ps->particles[i].position.y;
        vertices[i].color[0] = ps->particles[i].color[0];
        vertices[i].color[1] = ps->particles[i].color[1];
        vertices[i].color[2] = ps->particles[i].color[2];
        vertices[i].color[3] = ps->particles[i].color[3];
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleVertex) * ps->count, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(vertices);
}

void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Config* config) {
    if (!renderer || !renderer->initialized || !ps) return;
    
    // Fade previous frame
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(0);
    glColor4f(0.0f, 0.0f, 0.0f, 0.03f); // Subtle fade
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
    
    // Draw particles as lines (trails)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(1.0f);
    
    glBegin(GL_LINES);
    for (int i = 0; i < ps->count; i++) {
        Particle* p = &ps->particles[i];
        glColor4fv(p->color);
        glVertex2f(p->prev_position.x, p->prev_position.y);
        glVertex2f(p->position.x, p->position.y);
    }
    glEnd();
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
            shader_delete(&renderer->shader);
        }
        free(renderer);
    }
}
