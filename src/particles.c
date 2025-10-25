#include "particles.h"
#include "vector_field.h"
#include "camera.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Random float between 0 and 1
static float randf() {
    return (float)rand() / (float)RAND_MAX;
}

// Random float between min and max
static float randf_range(float min, float max) {
    return min + randf() * (max - min);
}

// Reset particle within camera view
static void particle_reset_in_view(Particle* p, const Config* config, const Camera* cam) {
    float left, right, bottom, top;
    camera_get_view_bounds(cam, &left, &right, &bottom, &top);
    
    // Spawn across the entire visible area (no margin reduction)
    p->position.x = randf_range(left, right);
    p->position.y = randf_range(bottom, top);
    p->prev_position = p->position;
    p->lifetime = 0.0f;
}


// Update particle color based on velocity
static void update_particle_color(Particle* p, vec2 velocity) {
    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
    speed = fminf(speed / 2.0f, 1.0f);
    
    if (speed < 0.5f) {
        float t = speed * 2.0f;
        p->color[0] = 0.0f;
        p->color[1] = 0.5f + t * 0.5f;
        p->color[2] = 1.0f;
    } else {
        float t = (speed - 0.5f) * 2.0f;
        p->color[0] = t;
        p->color[1] = 1.0f - t * 0.3f;
        p->color[2] = 1.0f - t;
    }
    
    p->color[3] = 0.3f;
}

// Check if particle is outside camera view
// Check if particle is outside camera view (with proper margin)
static bool is_particle_outside_view(const Particle* p, const Camera* cam) {
    float left, right, bottom, top;
    camera_get_view_bounds(cam, &left, &right, &bottom, &top);
    
    // Calculate width and height of view
    float view_width = right - left;
    float view_height = top - bottom;
    
    // Use percentage-based margin (10% of view size)
    float margin_x = view_width * 0.15f;
    float margin_y = view_height * 0.15f;
    
    return (p->position.x < left - margin_x || 
            p->position.x > right + margin_x ||
            p->position.y < bottom - margin_y || 
            p->position.y > top + margin_y);
}


// Create particle system
ParticleSystem* particle_system_create(int initial_capacity) {
    ParticleSystem* ps = (ParticleSystem*)malloc(sizeof(ParticleSystem));
    if (!ps) {
        printf("Error: Failed to allocate particle system\n");
        return NULL;
    }
    
    ps->particles = (Particle*)malloc(sizeof(Particle) * initial_capacity);
    if (!ps->particles) {
        printf("Error: Failed to allocate particles array\n");
        free(ps);
        return NULL;
    }
    
    ps->count = initial_capacity;
    ps->capacity = initial_capacity;
    
    srand((unsigned int)time(NULL));
    
    return ps;
}

// Initialize all particles within camera view
void particle_system_init_particles_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam) {
    for (int i = 0; i < ps->count; i++) {
        particle_reset_in_view(&ps->particles[i], config, cam);
        ps->particles[i].lifetime = randf() * config->particle_lifetime;
    }
}

// Destroy particle system
void particle_system_destroy(ParticleSystem* ps) {
    if (ps) {
        if (ps->particles) {
            free(ps->particles);
        }
        free(ps);
    }
}

// Resize particle system
void particle_system_resize(ParticleSystem* ps, int new_count) {
    if (new_count <= 0 || !ps) return;
    
    if (new_count > ps->capacity) {
        Particle* new_particles = (Particle*)realloc(ps->particles, sizeof(Particle) * new_count);
        if (!new_particles) {
            printf("Error: Failed to reallocate particles array\n");
            return;
        }
        ps->particles = new_particles;
        ps->capacity = new_count;
    }
    
    ps->count = new_count;
}

// Update all particles with camera-aware spawning
void particle_system_update_with_camera(ParticleSystem* ps, const Config* config, const Camera* cam, float dt) {
    if (!ps || config->paused) return;
    
    float adjusted_dt = dt * config->simulation_speed * config->integration_step;
    
    for (int i = 0; i < ps->count; i++) {
        Particle* p = &ps->particles[i];
        
        p->prev_position = p->position;
        vec2 velocity = vector_field_evaluate(p->position, config);
        update_particle_color(p, velocity);
        
        switch (config->integration_method) {
            case INTEGRATION_EULER:
                p->position.x += velocity.x * adjusted_dt;
                p->position.y += velocity.y * adjusted_dt;
                break;
            case INTEGRATION_RK2: {
                vec2 k1 = velocity;
                vec2 pos_mid;
                pos_mid.x = p->position.x + k1.x * adjusted_dt * 0.5f;
                pos_mid.y = p->position.y + k1.y * adjusted_dt * 0.5f;
                vec2 k2 = vector_field_evaluate(pos_mid, config);
                p->position.x += k2.x * adjusted_dt;
                p->position.y += k2.y * adjusted_dt;
                break;
            }
            case INTEGRATION_RK4: {
                vec2 pos_init = p->position;
                vec2 k1 = velocity;
                
                vec2 pos2 = {pos_init.x + k1.x * adjusted_dt * 0.5f, pos_init.y + k1.y * adjusted_dt * 0.5f};
                vec2 k2 = vector_field_evaluate(pos2, config);
                
                vec2 pos3 = {pos_init.x + k2.x * adjusted_dt * 0.5f, pos_init.y + k2.y * adjusted_dt * 0.5f};
                vec2 k3 = vector_field_evaluate(pos3, config);
                
                vec2 pos4 = {pos_init.x + k3.x * adjusted_dt, pos_init.y + k3.y * adjusted_dt};
                vec2 k4 = vector_field_evaluate(pos4, config);
                
                p->position.x = pos_init.x + (k1.x + 2.0f*k2.x + 2.0f*k3.x + k4.x) * adjusted_dt / 6.0f;
                p->position.y = pos_init.y + (k1.y + 2.0f*k2.y + 2.0f*k3.y + k4.y) * adjusted_dt / 6.0f;
                break;
            }
        }
        
        p->lifetime += adjusted_dt;
        
        // Respawn if outside camera view or lifetime exceeded
        if (is_particle_outside_view(p, cam) || p->lifetime > config->particle_lifetime) {
            particle_reset_in_view(p, config, cam);
        }
    }
}

// Redistribute particles when camera changes significantly
void particle_system_redistribute(ParticleSystem* ps, const Config* config, const Camera* cam) {
    // Respawn all particles within current camera view
    for (int i = 0; i < ps->count; i++) {
        particle_reset_in_view(&ps->particles[i], config, cam);
        // Randomize lifetime so they don't all die at once
        ps->particles[i].lifetime = randf() * config->particle_lifetime * 0.5f;
    }
}

// Reset all particles
void particle_system_reset(ParticleSystem* ps, const Config* config, Camera* cam) {
    particle_system_init_particles_with_camera(ps, config, cam);
}