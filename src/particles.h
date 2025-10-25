#ifndef PARTICLES_H
#define PARTICLES_H

#include "vector_field.h"
#include "config.h"
#include "camera.h"
#include <stdbool.h>

// Particle structure
typedef struct {
    vec2 position;
    vec2 prev_position;  // Add this
    vec2 velocity;
    float color[4];
    float lifetime;
} Particle;

// Particle system structure
typedef struct {
    Particle* particles;
    int count;
    int capacity;
    
    // Spawn area bounds (normalized coordinates -1 to 1)
    float spawn_min_x;
    float spawn_max_x;
    float spawn_min_y;
    float spawn_max_y;
} ParticleSystem;

// Particle system functions
ParticleSystem* particle_system_create(int initial_capacity);
void particle_system_init_particles_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam);
void particle_system_update_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam, float dt);
void particle_system_destroy(ParticleSystem* ps);
void particle_system_resize(ParticleSystem* ps, int new_count);
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam);
void particle_system_reset(ParticleSystem* ps, const Config* config, Camera* cam);

// Particle helper functions
void particle_spawn(Particle* p, const ParticleSystem* ps, const Config* config);
void particle_respawn_if_dead(Particle* p, const ParticleSystem* ps, const Config* config);

// Integration methods
void integrate_euler(Particle* p, const Config* config, float dt);
void integrate_rk2(Particle* p, const Config* config, float dt);
void integrate_rk4(Particle* p, const Config* config, float dt);

#endif // PARTICLES_H
