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

// Fast inverse square root (pure magic)
static inline float fast_inv_sqrt(float x) {
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x * (1.5f - xhalf * x * x);
    return x;
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
static void particle_reset_in_view(Particle* p, const Config* config, const ViewCache* cache) {
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
    
    ps->particles = (Particle*)malloc(sizeof(Particle) * initial_capacity);
    if (!ps->particles) {
        free(ps);
        fprintf(stderr, "Error: Failed to allocate particles\n");
        return NULL;
    }
    
    ps->count = initial_capacity;
    ps->capacity = initial_capacity;
    ps->target_count = initial_capacity;
    
    srand((unsigned int)time(NULL));
    return ps;
}

void particle_system_resize(ParticleSystem* ps, int new_count) {
    if (new_count <= 0 || !ps) return;
    
    // Use growth factor to reduce realloc calls (1.5x strategy)
    if (new_count > ps->capacity) {
        int new_capacity = (int)(new_count * 1.5f);
        Particle* new_particles = (Particle*)realloc(ps->particles, sizeof(Particle) * new_capacity);
        if (!new_particles) {
            fprintf(stderr, "Error: Failed to resize particle array\n");
            return;
        }
        ps->particles = new_particles;
        ps->capacity = new_capacity;
    }
    
    ps->count = new_count;
}

// Random redistribution
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam) {
    ViewCache cache;
    build_view_cache(&cache, cam);
    
    float lifetime_mult = config->particle_lifetime * 0.5f;
    
    for (int i = 0; i < ps->count; i++) {
        particle_reset_in_view(&ps->particles[i], config, &cache);
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
    if (!ps || config->paused) return;
    
    // Build view cache once for entire frame
    ViewCache cache;
    build_view_cache(&cache, cam);
    
    float adaptive_step = config->integration_step / cam->zoom;
    float adjusted_dt = dt * config->simulation_speed * adaptive_step;
    
    // CRITICAL: Calculate particles per unit area for UNIFORM density
    float area = cache.view_width * cache.view_height;
    float desired_density = ps->count / area;  // particles per square unit
    
    int forced_respawns_per_frame = (int)(ps->count * 0.005f);  // 0.5% per frame
    if (forced_respawns_per_frame < 1) forced_respawns_per_frame = 1;
    
    int respawn_counter = 0;
    
    // Process all particles
    for (int i = 0; i < ps->count; i++) {
        Particle* p = &ps->particles[i];
        
        // Save previous position BEFORE integration
        p->prev_position = p->position;
        
        // Evaluate vector field
        vec2 velocity = vector_field_evaluate(p->position, config);
        update_particle_color(p, velocity);
        
        // RK4 Integration
        float x0 = p->position.x;
        float y0 = p->position.y;
        
        vec2 k1 = velocity;
        float dt_half = adjusted_dt * 0.5f;
        
        vec2 pos2 = {x0 + k1.x * dt_half, y0 + k1.y * dt_half};
        vec2 k2 = vector_field_evaluate(pos2, config);
        
        vec2 pos3 = {x0 + k2.x * dt_half, y0 + k2.y * dt_half};
        vec2 k3 = vector_field_evaluate(pos3, config);
        
        vec2 pos4 = {x0 + k3.x * adjusted_dt, y0 + k3.y * adjusted_dt};
        vec2 k4 = vector_field_evaluate(pos4, config);
        
        float dt_sixth = adjusted_dt * 0.16666667f;
        p->position.x = x0 + (k1.x + 2.0f*k2.x + 2.0f*k3.x + k4.x) * dt_sixth;
        p->position.y = y0 + (k1.y + 2.0f*k2.y + 2.0f*k3.y + k4.y) * dt_sixth;
        
        p->lifetime += adjusted_dt;
        
        // Check if particle needs respawning
        bool outside = is_particle_outside_view(p, &cache);
        bool expired = p->lifetime > config->particle_lifetime;
        
        // CRITICAL: Force continuous uniform respawning
        // This prevents clustering in stable flow regions
        bool force_respawn = (respawn_counter < forced_respawns_per_frame) && 
                             (randf() < 0.01f);
        
        if (outside || expired || force_respawn) {
            p->position.x = cache.left + randf() * cache.view_width;
            p->position.y = cache.bottom + randf() * cache.view_height;
            p->prev_position = p->position;
            
            // Random lifetime to prevent synchronization
            p->lifetime = randf() * config->particle_lifetime * 0.2f;
            
            if (force_respawn) respawn_counter++;
        }
    }
}

// Calculate target particle count based on visible area
void particle_system_adjust_count_for_zoom(ParticleSystem* ps, const Config* config, const Camera* cam) {
    ViewCache cache;
    build_view_cache(&cache, cam);
    
    float visible_area = cache.view_width * cache.view_height;
    
    // Base density: particles per unit area
    float base_density = config->particle_count * 0.25f; // 0.25 = 1/4.0
    
    int target = (int)(base_density * visible_area);
    
    // Clamp to reasonable bounds
    if (target < 1000) target = 1000;
    if (target > 200000) target = 200000;
    
    ps->target_count = target;
    
    // Only resize if difference is significant (>20%)
    int diff = abs(ps->count - target);
    if (diff > ps->count / 5) {  // 1/5 = 20%
#ifdef DEBUG
        printf("Adjusted particle count: %d (zoom: %.2f)\n", target, cam->zoom);
#endif
        particle_system_resize(ps, target);
    }
}

void particle_system_destroy(ParticleSystem* ps) {
    if (ps) {
        if (ps->particles) free(ps->particles);
        free(ps);
    }
}