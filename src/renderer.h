#ifndef RENDERER_H
#define RENDERER_H

#include "particles.h"
#include "config.h"
#include "shader.h"
#include "camera.h"

#include <stdbool.h>

// Renderer structure
typedef struct {
    unsigned int vao;
    unsigned int vbo;
    ShaderProgram shader;
    bool initialized;
} Renderer;

// Renderer functions
Renderer* renderer_create();
bool renderer_init(Renderer* renderer, int window_width, int window_height);
void renderer_update_particles(Renderer* renderer, const ParticleSystem* ps);
void renderer_draw(Renderer* renderer, const ParticleSystem* ps, const Config* config, const Camera* cam);
void renderer_set_viewport(Renderer* renderer, int width, int height);
void renderer_destroy(Renderer* renderer);

#endif // RENDERER_H
