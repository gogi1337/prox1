#ifndef RENDERER_H
#define RENDERER_H

#include "particles.h"
#include "config.h"
#include "shader.h"
#include "camera.h"

#include <stdbool.h>
#include <GL/gl.h>

// Renderer structure
typedef struct {
    // Particle rendering
    unsigned int vao;
    unsigned int vbo;
    
    // Fade effect rendering
    unsigned int fade_vao;
    unsigned int fade_vbo;
    
    // Shader programs (separate for particles and fade)
    ShaderProgram particle_shader;
    ShaderProgram fade_shader;
    
    // Particle shader uniform locations
    int particle_projection_loc;
    
    // Fade shader uniform locations
    int fade_projection_loc;
    int fade_color_loc;
    
    // Rendering state
    int particle_count;
    bool initialized;
    bool should_clear;
} Renderer;

// Renderer functions
Renderer* renderer_create();
bool renderer_init(Renderer* renderer, int window_width, int window_height);
void renderer_update_particles(Renderer* renderer, const ParticleSystem* ps);
void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Config* config, const Camera* cam);
void renderer_set_viewport(Renderer* renderer, int width, int height);
void renderer_request_clear(Renderer* renderer);
void renderer_destroy(Renderer* renderer);

#endif // RENDERER_H