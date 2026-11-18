#ifndef PARTICLES_H
#define PARTICLES_H

#include "config.h"
#include "vector_field.h"
#include "camera.h"
#include <stdbool.h>

// Single particle data
typedef struct {
    vec2 position;       // Current position in world space
    vec2 prev_position;  // Previous position (for trail rendering)
    float color[4];      // RGBA color (computed from velocity magnitude)
    float lifetime;      // Current age of particle (seconds)
} Particle;

// Dynamic particle system
typedef struct {
    Particle* particles;  // Dynamic array of particles
    int count;           // Number of particles
} ParticleSystem;

// View cache
typedef struct {
    float left, right, bottom, top;
    float view_width, view_height;
    float margin_x, margin_y;
} ViewCache;

ParticleSystem* particle_system_create(int initial_capacity);
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam);
void particle_system_redistribute_grid(ParticleSystem* ps, const Config* config, const Camera* cam);
void particle_system_update(ParticleSystem* ps, const Config* config, const Camera* cam, float dt);
void particle_system_destroy(ParticleSystem* ps);

#endif // PARTICLES_H
