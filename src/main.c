#define RGFW_IMPLEMENTATION
#define RGFW_BUFFER
#define RGFW_OPENGL
#define RGFW_PRINT_ERRORS
#define RGFW_DEBUG
#include "../ext/RGFW.h"

#ifndef __EMSCRIPTEN__
#define RGL_LOAD_IMPLEMENTATION
#include "../ext/rgload.h"
#endif

#include "config.h"
#include "particles.h"
#include "vector_field.h"
#include "renderer.h"
#include <stdio.h>
#include <time.h>

// Calculate delta time
float get_delta_time() {
    static clock_t last_time = 0;
    clock_t current_time = clock();
    
    if (last_time == 0) {
        last_time = current_time;
        return 0.016f; // Default to ~60 FPS
    }
    
    float delta = (float)(current_time - last_time) / CLOCKS_PER_SEC;
    last_time = current_time;
    
    return delta;
}

// Handle keyboard input
void handle_input(RGFW_window* win, RGFW_keyEvent* event, Config* config, ParticleSystem* ps) {
    switch (event->value) {
        case RGFW_space:
            // Toggle pause
            config->paused = !config->paused;
            printf("Simulation %s\n", config->paused ? "paused" : "resumed");
            break;
            
        case RGFW_r:
            // Reset particles
            particle_system_reset(ps, config);
            printf("Particles reset\n");
            break;
            
        case RGFW_1:
        case RGFW_2:
        case RGFW_3:
        case RGFW_4:
        case RGFW_5:
            // Switch vector field
            config->vector_field_type = (VectorFieldType)(event->value - RGFW_1);
            particle_system_reset(ps, config);
            printf("Switched to vector field: %d\n", config->vector_field_type);
            break;
            
        case RGFW_up:
            // Increase particle count
            config->particle_count += 500;
            if (config->particle_count > 50000) config->particle_count = 50000;
            particle_system_resize(ps, config->particle_count);
            particle_system_init_particles(ps, config);
            printf("Particle count: %d\n", config->particle_count);
            break;
            
        case RGFW_down:
            // Decrease particle count
            config->particle_count -= 500;
            if (config->particle_count < 100) config->particle_count = 100;
            particle_system_resize(ps, config->particle_count);
            particle_system_init_particles(ps, config);
            printf("Particle count: %d\n", config->particle_count);
            break;
            
        case RGFW_escape:
            // Exit
            RGFW_window_close(win);
            break;
    }
}
int main(void) {
    printf("prox1\n");
    
    // Load or create default configuration
    Config config = config_create_default();
    config_load_from_file(&config, "config.ini");
    config_print(&config);
    
    // Create RGFW window
    RGFW_window* win = RGFW_createWindow(
        "prox1",
        0, 0, config.window_width, config.window_height,
        RGFW_windowCenter | RGFW_windowAllowDND | RGFW_windowOpenGL
    );
    
    if (!win) {
        printf("Error: Failed to create window\n");
        return 1;
    }
    
    Renderer* renderer = renderer_create();
    if (!renderer_init(renderer, config.window_width, config.window_height)) {
        printf("Error: Failed to initialize renderer\n");
        RGFW_window_close(win);
        return 1;
    }
    
    ParticleSystem* ps = particle_system_create(config.particle_count);
    if (!ps) {
        printf("Error: Failed to create particle system\n");
        renderer_destroy(renderer);
        RGFW_window_close(win);
        return 1;
    }
    
    particle_system_init_particles(ps, &config);
    
    printf("SPACE   - Pause/Resume\n");
    printf("R       - Reset particles\n");
    printf("1-5     - Switch vector field\n");
    printf("UP/DOWN - Increase/Decrease particle count\n");
    printf("ESC     - Exit\n");
    
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        float dt = get_delta_time();
        
        RGFW_event event;
        while (RGFW_window_checkEvent(win, &event)) {
            if (event.type == RGFW_quit) {
                break;
            }
            if (event.type == RGFW_keyPressed) {
                handle_input(win, &event, &config, ps);
            }
        }
        
        particle_system_update(ps, &config, dt);
        renderer_update_particles(renderer, ps);
        renderer_draw(renderer, ps, &config);
        RGFW_window_swapBuffers_OpenGL(win);
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    particle_system_destroy(ps);
    renderer_destroy(renderer);
    RGFW_window_close(win);
    
    // Save configuration
    config_save_to_file(&config, "config.ini");

    return 0;
}