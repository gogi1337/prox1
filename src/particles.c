#include "particles.h"
#include "vector_field.h"
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

// Reset a single particle to random position
static void particle_reset(Particle* p, const Config* config) {
    p->position.x = randf_range(-2.0f, 2.0f);
    p->position.y = randf_range(-2.0f, 2.0f);
    p->prev_position = p->position;  // Initialize prev position
    p->lifetime = 0.0f;
}

// Update particle color based on velocity
static void update_particle_color(Particle* p, vec2 velocity) {
    // Calculate velocity magnitude
    float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
    
    // Normalize speed to 0-1 range (adjust based on your field scale)
    speed = fminf(speed / 5.0f, 1.0f);
    
    // Create color gradient: slow=blue, medium=cyan, fast=yellow/red
    if (speed < 0.5f) {
        // Blue to cyan
        float t = speed * 2.0f;
        p->color[0] = 0.0f;              // R
        p->color[1] = 0.5f + t * 0.5f;   // G
        p->color[2] = 1.0f;               // B
    } else {
        // Cyan to yellow/red
        float t = (speed - 0.5f) * 2.0f;
        p->color[0] = t;                  // R
        p->color[1] = 1.0f - t * 0.3f;   // G
        p->color[2] = 1.0f - t;          // B
    }
    
    p->color[3] = 0.3f; // Alpha
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

// Initialize all particles
void particle_system_init_particles(ParticleSystem* ps, const Config* config) {
    for (int i = 0; i < ps->count; i++) {
        particle_reset(&ps->particles[i], config);
        ps->particles[i].lifetime = randf() * config->particle_lifetime;
    }
}

// Update all particles
void particle_system_update(ParticleSystem* ps, const Config* config, float dt) {
    if (!ps || config->paused) return;
    
    float adjusted_dt = dt * config->simulation_speed * config->integration_step;
    
    for (int i = 0; i < ps->count; i++) {
        Particle* p = &ps->particles[i];
        
        // Save previous position for line rendering
        p->prev_position = p->position;
        
        // Get velocity at current position
        vec2 velocity = vector_field_evaluate(p->position, config);
        
        // Update color based on velocity
        update_particle_color(p, velocity);
        
        // Integrate position based on method
        switch (config->integration_method) {
            case INTEGRATION_EULER: {
                p->position.x += velocity.x * adjusted_dt;
                p->position.y += velocity.y * adjusted_dt;
                break;
            }
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
                
                vec2 pos2;
                pos2.x = pos_init.x + k1.x * adjusted_dt * 0.5f;
                pos2.y = pos_init.y + k1.y * adjusted_dt * 0.5f;
                vec2 k2 = vector_field_evaluate(pos2, config);
                
                vec2 pos3;
                pos3.x = pos_init.x + k2.x * adjusted_dt * 0.5f;
                pos3.y = pos_init.y + k2.y * adjusted_dt * 0.5f;
                vec2 k3 = vector_field_evaluate(pos3, config);
                
                vec2 pos4;
                pos4.x = pos_init.x + k3.x * adjusted_dt;
                pos4.y = pos_init.y + k3.y * adjusted_dt;
                vec2 k4 = vector_field_evaluate(pos4, config);
                
                p->position.x = pos_init.x + (k1.x + 2.0f*k2.x + 2.0f*k3.x + k4.x) * adjusted_dt / 6.0f;
                p->position.y = pos_init.y + (k1.y + 2.0f*k2.y + 2.0f*k3.y + k4.y) * adjusted_dt / 6.0f;
                break;
            }
        }
        
        // Update lifetime
        p->lifetime += adjusted_dt;
        
        // Reset if too old or WAY out of bounds (change 2.0f to 4.0f)
        if (p->lifetime > config->particle_lifetime || 
            fabsf(p->position.x) > 10.0f ||   // Much larger bounds
            fabsf(p->position.y) > 10.0f) {
            particle_reset(p, config);
        }

    }
}

// Reset all particles
void particle_system_reset(ParticleSystem* ps, const Config* config) {
    particle_system_init_particles(ps, config);
}