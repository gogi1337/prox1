#define GL_GLEXT_PROTOTYPES
#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glext.h>

// Helper function to check OpenGL errors (debug only)
static void check_gl_error(const char* operation) {
    (void)operation;
#ifdef DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL Error after %s: 0x%x\n", operation, error);
    }
#endif
}

// (Re)create the offscreen scene texture at the given size. Contents are lost.
static bool scene_create(Renderer* renderer, int w, int h) {
    if (!renderer->scene_fbo) glGenFramebuffers(1, &renderer->scene_fbo);
    if (!renderer->scene_tex) glGenTextures(1, &renderer->scene_tex);

    // 8-bit on purpose: the glow of the trails comes from 8-bit rounding in the
    // fade; a 16-bit scene was about 10x dimmer.
    glBindTexture(GL_TEXTURE_2D, renderer->scene_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->scene_fbo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->scene_tex, 0);
    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!complete) {
        fprintf(stderr, "Error: Could not create the offscreen scene buffer\n");
        return false;
    }
    renderer->scene_w = w;
    renderer->scene_h = h;
    renderer->should_clear = true;
    return true;
}

Renderer* renderer_create() {
    Renderer* renderer = (Renderer*)calloc(1, sizeof(Renderer));
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
    
    if (!scene_create(renderer, window_width, window_height)) return false;
    
    renderer->initialized = true;
    
    printf("Renderer initialized successfully\n");
    printf("Particle shader ID: %d\n", renderer->particle_shader.program_id);
    printf("Fade shader ID: %d\n", renderer->fade_shader.program_id);
    
    return true;
}

void renderer_update_particles(Renderer* renderer, ParticleSystem* ps) {
    if (!renderer || !renderer->initialized || !ps || ps->count == 0 || !ps->dirty) return;
    
    // Allocate vertex data (2 vertices per particle for line rendering)
    int needed = ps->count * 2;
    if (needed > renderer->vertex_capacity) {
        ParticleVertex* grown = (ParticleVertex*)realloc(renderer->vertices, sizeof(ParticleVertex) * needed);
        if (!grown) {
            fprintf(stderr, "Error: Failed to allocate particle vertices\n");
            return;
        }
        renderer->vertices = grown;
        renderer->vertex_capacity = needed;
    }
    ParticleVertex* vertices = renderer->vertices;
    
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
    ps->dirty = false;
    
    check_gl_error("Update particles");
}

void renderer_request_clear(Renderer* renderer) {
    if (renderer) {
        renderer->should_clear = true;
    }
}

void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Camera* cam) {
    if (!renderer || !renderer->initialized || !ps) return;
    
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->scene_fbo);
    glViewport(0, 0, renderer->scene_w, renderer->scene_h);
    glEnable(GL_BLEND);
    
    // Handle clear request
    if (renderer->should_clear) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        renderer->should_clear = false;
    } else {
        glUseProgram(renderer->fade_shader.program_id);
        // Color fades; alpha channel is left alone so the window stays opaque
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
        
        glBindVertexArray(renderer->fade_vao);
        
        // Fixed fade per frame (8%): this sets the trail length
        float fade_color[4] = {0.0f, 0.0f, 0.0f, 0.08f};
        glUniform4fv(renderer->fade_color_loc, 1, fade_color);
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        check_gl_error("Draw fade");
    }
    
    if (renderer->particle_count > 0) {
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
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
        
        glUniformMatrix4fv(renderer->particle_projection_loc, 1, GL_FALSE, projection);
        
        glBindVertexArray(renderer->vao);
        glLineWidth(1.5f);
        
        // Draw all particle trails as lines
        glDrawArrays(GL_LINES, 0, renderer->particle_count);
        
        check_gl_error("Draw particles");
        
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    // Copy the finished scene to the window
    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->scene_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, renderer->scene_w, renderer->scene_h,
                      0, 0, renderer->scene_w, renderer->scene_h,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Window was resized: the scene buffer must match it (trails start over)
void renderer_set_viewport(Renderer* renderer, int width, int height) {
    if (!renderer || !renderer->initialized || width <= 0 || height <= 0) return;
    if (width != renderer->scene_w || height != renderer->scene_h) {
        scene_create(renderer, width, height);
    }
}

void renderer_destroy(Renderer* renderer) {
    if (renderer) {
        if (renderer->initialized) {
            glDeleteVertexArrays(1, &renderer->vao);
            glDeleteBuffers(1, &renderer->vbo);
            glDeleteVertexArrays(1, &renderer->fade_vao);
            glDeleteBuffers(1, &renderer->fade_vbo);
            glDeleteFramebuffers(1, &renderer->scene_fbo);
            glDeleteTextures(1, &renderer->scene_tex);
            shader_delete(&renderer->particle_shader);
            shader_delete(&renderer->fade_shader);
        }
        free(renderer->vertices);
        free(renderer);
    }
}