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
    vec2 velocity;       // Current velocity (not actively used, can be removed if unused)
    float color[4];      // RGBA color (computed from velocity magnitude)
    float lifetime;      // Current age of particle (seconds)
} Particle;

// Dynamic particle system
typedef struct {
    Particle* particles;  // Dynamic array of particles
    int count;           // Current number of active particles
    int capacity;        // Allocated capacity (may be > count)
    int target_count;    // Target count based on zoom level
} ParticleSystem;

// View cache
typedef struct {
    float left, right, bottom, top;
    float view_width, view_height;
    float margin_x, margin_y;
} ViewCache;

// Create a new particle system with initial capacity
// Returns NULL on allocation failure
ParticleSystem* particle_system_create(int initial_capacity);

// Free all memory associated with particle system
void particle_system_destroy(ParticleSystem* ps);

// Resize particle array to new count (handles reallocation if needed)
void particle_system_resize(ParticleSystem* ps, int new_count);

// Initialize particles with grid-based distribution in camera view
void particle_system_init_particles_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam);

// Main update function: integrates particle positions and handles respawning
// Uses adaptive time-stepping based on camera zoom
void particle_system_update_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam, float dt);

// Redistribute particles randomly within camera view
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam);

// Redistribute particles in a grid pattern (more even distribution)
void particle_system_redistribute_grid(ParticleSystem* ps, const Config* config, const Camera* cam);

// Dynamically adjust particle count based on visible area (zoom level)
void particle_system_adjust_count_for_zoom(ParticleSystem* ps, const Config* config, const Camera* cam);

// Reset all particles (uses grid distribution)
void particle_system_reset(ParticleSystem* ps, const Config* config, const Camera* cam);

#endif // PARTICLES_H
