#ifndef RENDERER_H
#define RENDERER_H

#include "particles.h"
#include "config.h"
#include "shader.h"
#include "camera.h"

#include <stdbool.h>
#include <GL/gl.h>

// Vertex data structure for GPU
typedef struct {
    float position[2];
    float color[4];
} ParticleVertex;

// Renderer structure
typedef struct {
    // Particle rendering
    unsigned int vao;
    unsigned int vbo;
    
    // Fade effect rendering
    unsigned int fade_vao;
    unsigned int fade_vbo;
    
    // Offscreen scene: trails accumulate here, then it is copied to the window.
    // The window's own buffers are never relied on to keep their contents.
    unsigned int scene_fbo;
    unsigned int scene_tex;
    int scene_w, scene_h;

    // Shader programs (separate for particles and fade)
    ShaderProgram particle_shader;
    ShaderProgram fade_shader;
    
    // Particle shader uniform locations
    int particle_projection_loc;
    
    // Fade shader uniform locations
    int fade_color_loc;
    
    // Rendering state
    int particle_count;
    ParticleVertex* vertices;  // CPU staging buffer, reused every frame
    int vertex_capacity;       // Size of vertices in elements
    bool initialized;
    bool should_clear;
} Renderer;

Renderer* renderer_create();
bool renderer_init(Renderer* renderer, int window_width, int window_height);
void renderer_update_particles(Renderer* renderer, ParticleSystem* ps);  // no-op unless ps->dirty
void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Camera* cam);
void renderer_set_viewport(Renderer* renderer, int width, int height);
void renderer_request_clear(Renderer* renderer);
void renderer_destroy(Renderer* renderer);

#endif // RENDERER_H