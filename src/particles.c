#include "particles.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Random float between 0 and 1
static inline float randf() {
    return (float)rand() / (float)RAND_MAX;
}

// Random float between min and max
static inline float randf_range(float min, float max) {
    return min + randf() * (max - min);
}

// Build view cache once per frame
static inline void build_view_cache(ViewCache* cache, const Camera* cam) {
    camera_get_view_bounds(cam, &cache->left, &cache->right, &cache->bottom, &cache->top);
    cache->view_width = cache->right - cache->left;
    cache->view_height = cache->top - cache->bottom;
    cache->margin_x = cache->view_width * 0.15f;
    cache->margin_y = cache->view_height * 0.15f;
}

// Random spawn within camera view
static void particle_reset_in_view(Particle* p, const ViewCache* cache) {
    p->position.x = randf_range(cache->left, cache->right);
    p->position.y = randf_range(cache->bottom, cache->top);
    p->prev_position = p->position;
    p->lifetime = 0.0f;
}

// Update particle color based on velocity
static inline void update_particle_color(Particle* p, vec2 velocity) {
    // Use squared speed to avoid sqrt
    float speed_sq = velocity.x * velocity.x + velocity.y * velocity.y;
    
    // Normalize against squared threshold (2.0^2 = 4.0)
    float speed_normalized = fminf(speed_sq * 0.25f, 1.0f);  // *0.25 = /4.0
    
    if (speed_normalized < 0.5f) {
        float t = speed_normalized * 2.0f;
        p->color[0] = 0.0f;
        p->color[1] = 0.5f + t * 0.5f;
        p->color[2] = 1.0f;
    } else {
        float t = (speed_normalized - 0.5f) * 2.0f;
        p->color[0] = t;
        p->color[1] = 1.0f - t * 0.3f;
        p->color[2] = 1.0f - t;
    }
    
    p->color[3] = 0.3f;
}

// Check if particle is outside camera view (uses cached bounds)
static inline bool is_particle_outside_view(const Particle* p, const ViewCache* cache) {
    return (p->position.x < cache->left - cache->margin_x || 
            p->position.x > cache->right + cache->margin_x ||
            p->position.y < cache->bottom - cache->margin_y || 
            p->position.y > cache->top + cache->margin_y);
}

// Create particle system
ParticleSystem* particle_system_create(int initial_capacity) {
    ParticleSystem* ps = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    if (!ps) {
        fprintf(stderr, "Error: Failed to allocate particle system\n");
        return NULL;
    }
    
    ps->particles = (Particle*)calloc(initial_capacity, sizeof(Particle));
    if (!ps->particles) {
        free(ps);
        fprintf(stderr, "Error: Failed to allocate particles\n");
        return NULL;
    }
    
    ps->count = initial_capacity;
    
    srand((unsigned int)time(NULL));
    return ps;
}

// Random redistribution
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam) {
    ViewCache cache;
    build_view_cache(&cache, cam);
    
    float lifetime_mult = config->particle_lifetime * 0.5f;
    
    for (int i = 0; i < ps->count; i++) {
        particle_reset_in_view(&ps->particles[i], &cache);
        ps->particles[i].lifetime = randf() * lifetime_mult;
    }
}

// Grid-based redistribution
void particle_system_redistribute_grid(ParticleSystem* ps, const Config* config, const Camera* cam) {
    ViewCache cache;
    build_view_cache(&cache, cam);
    
    int grid_size = (int)sqrtf((float)ps->count);
    
    // Pre-calculate lifetime multiplier
    float lifetime_mult = config->particle_lifetime * 0.3f;
    
    int idx = 0;
    
    // Use Poisson disk-like distribution for better spacing
    for (int i = 0; i < grid_size && idx < ps->count; i++) {
        for (int j = 0; j < grid_size && idx < ps->count; j++) {
            float step_x = cache.view_width / grid_size;
            float step_y = cache.view_height / grid_size;
            
            // Jittered grid: add random offset from cell center
            float jitter_x = (randf() - 0.5f) * step_x * 0.8f;
            float jitter_y = (randf() - 0.5f) * step_y * 0.8f;
            
            ps->particles[idx].position.x = cache.left + (i + 0.5f) * step_x + jitter_x;
            ps->particles[idx].position.y = cache.bottom + (j + 0.5f) * step_y + jitter_y;
            ps->particles[idx].prev_position = ps->particles[idx].position;
            ps->particles[idx].lifetime = randf() * lifetime_mult;
            
            idx++;
        }
    }
    
    // Fill remaining particles with pure random distribution
    while (idx < ps->count) {
        ps->particles[idx].position.x = randf_range(cache.left, cache.right);
        ps->particles[idx].position.y = randf_range(cache.bottom, cache.top);
        ps->particles[idx].prev_position = ps->particles[idx].position;
        ps->particles[idx].lifetime = randf() * lifetime_mult;
        idx++;
    }
}

// Main update with adaptive integration
void particle_system_update(ParticleSystem* ps, const Config* config, const Camera* cam, float dt) {
    if (!ps || ps->count <= 0 || config->paused) return;

    VectorFieldFunc field = vector_field_get(config->vector_field_num);
    if (!field) return;
    const float scale = config->field_scale;

    // Build view cache once for entire frame
    ViewCache cache;
    build_view_cache(&cache, cam);

    float sim_dt = dt * config->simulation_speed;
    float adjusted_dt = sim_dt * config->integration_step / cam->zoom;

    for (int i = 0; i < ps->count; i++) {
        Particle* p = &ps->particles[i];

        // Save previous position BEFORE integration
        p->prev_position = p->position;

        vec2 velocity = field(p->position, scale);
        update_particle_color(p, velocity);

        // RK4 Integration
        float x0 = p->position.x;
        float y0 = p->position.y;

        vec2 k1 = velocity;
        float dt_half = adjusted_dt * 0.5f;

        vec2 pos2 = {x0 + k1.x * dt_half, y0 + k1.y * dt_half};
        vec2 k2 = field(pos2, scale);

        vec2 pos3 = {x0 + k2.x * dt_half, y0 + k2.y * dt_half};
        vec2 k3 = field(pos3, scale);

        vec2 pos4 = {x0 + k3.x * adjusted_dt, y0 + k3.y * adjusted_dt};
        vec2 k4 = field(pos4, scale);

        float dt_sixth = adjusted_dt * 0.16666667f;
        p->position.x = x0 + (k1.x + 2.0f*k2.x + 2.0f*k3.x + k4.x) * dt_sixth;
        p->position.y = y0 + (k1.y + 2.0f*k2.y + 2.0f*k3.y + k4.y) * dt_sixth;

        // Lifetime is real simulation seconds, independent of zoom and step size
        p->lifetime += sim_dt;

        if (is_particle_outside_view(p, &cache) || p->lifetime > config->particle_lifetime) {
            particle_reset_in_view(p, &cache);
            // Random lifetime to prevent synchronization
            p->lifetime = randf() * config->particle_lifetime * 0.2f;
        }
    }

    // Force 0.5% of random particles to respawn each frame so they cannot
    // clump in stable flow regions. Random indices keep this uniform.
    int forced = ps->count / 200;
    if (forced < 1) forced = 1;
    for (int k = 0; k < forced; k++) {
        Particle* p = &ps->particles[rand() % ps->count];
        particle_reset_in_view(p, &cache);
        p->lifetime = randf() * config->particle_lifetime * 0.2f;
    }
}

void particle_system_destroy(ParticleSystem* ps) {
    if (ps) {
        if (ps->particles) free(ps->particles);
        free(ps);
    }
}