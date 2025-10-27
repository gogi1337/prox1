#define RGFW_IMPLEMENTATION
#define RGFW_BUFFER
#define RGFW_OPENGL
#define RGFW_PRINT_ERRORS
#define RGFW_DEBUG
#define GL_SILENCE_DEPRECATION
#include "../ext/RGFW.h"

#ifndef __EMSCRIPTEN__
#define RGL_LOAD_IMPLEMENTATION
#include "../ext/rgload.h"
#endif

#include "config.h"
#include "particles.h"
#include "vector_field.h"
#include "renderer.h"
#include "camera.h"

#include <stdio.h>
#include <time.h>
#include <math.h>

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
void handle_input(RGFW_window* win, RGFW_keyEvent* event, Config* config, Renderer* renderer, ParticleSystem* ps, Camera* camera) {
    switch (event->value) {
        case RGFW_space:
            // Toggle pause
            config->paused = !config->paused;
            printf("Simulation %s\n", config->paused ? "paused" : "resumed");
            break;
            
        case RGFW_r:
            // Reset particles
            particle_system_redistribute_grid(ps, config, camera);
            printf("Particles reset\n");
            break;
            
        case RGFW_1:
        case RGFW_2:
        case RGFW_3:
        case RGFW_4:
        case RGFW_5:
        case RGFW_6:
        case RGFW_7:
        case RGFW_8:
        case RGFW_9:
            config->vector_field_num = event->value - RGFW_1;
            printf("%d", event->value - RGFW_1);
            particle_system_redistribute(ps, config, camera);
            renderer_request_clear(renderer);  // Clear on next frame
            printf("Vector field: %d\n", config->vector_field_num);
            break;

        case RGFW_equals:  // + key
        case RGFW_kpPlus: {
            float old_zoom = camera->zoom;
            camera_zoom_in(camera);
            // Always redistribute on any zoom change
            if (camera->zoom != old_zoom) {
                particle_system_redistribute(ps, config, camera);
                printf("Zoom: %.2f (redistributed)\n", camera->zoom);
            }
            break;
        }

        case RGFW_minus:
        case RGFW_kpMinus: {
            float old_zoom = camera->zoom;
            camera_zoom_out(camera);
            // Always redistribute on any zoom change
            if (camera->zoom != old_zoom) {
                particle_system_redistribute(ps, config, camera);
                printf("Zoom: %.2f (redistributed)\n", camera->zoom);
            }
            break;
        }
        // Camera pan (WASD or Arrow keys)
        case RGFW_w:
        case RGFW_up:
            camera_pan(camera, 0, 0.1f);
            break;
        case RGFW_s:
        case RGFW_down:
            camera_pan(camera, 0, -0.1f);
            break;
        case RGFW_a:
        case RGFW_left:
            camera_pan(camera, -0.1f, 0);
            break;
        case RGFW_d:
        case RGFW_right:
            camera_pan(camera, 0.1f, 0);
            break;
        
        // Reset camera
        case RGFW_c:
            camera_reset(camera);
            printf("Camera reset\n");
            break;
            
        case RGFW_escape:
            // Exit
            win->internal.shouldClose = 1;
            break;
    }
}
int main(void) {
    printf("prox1\n");
    
    // Load or create default configuration
    Config config = config_create_default();
    config_load_from_file(&config, "config.ini");
    config_print(&config);

    // OpenGL version
    // RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    // hints->major = 3;
    // hints->minor = 3;
    // RGFW_setGlobalHints_OpenGL(hints);
    
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

    RGFW_window_swapInterval_OpenGL(win, 1); // VSync

    Camera camera = camera_create();
    
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
    
    particle_system_redistribute_grid(ps, &config, &camera);
    
    printf("SPACE   - Pause/Resume\n");
    printf("R       - Reset particles\n");
    printf("1-5     - Switch vector field\n");
    printf("W/A/S/D - Camera movement\n");
    printf("+/-     - Zoom / Outzoom \n");
    printf("C       - Reset camera \n");
    printf("ESC     - Exit\n");
    
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        float dt = get_delta_time();

        RGFW_event event;
        while (RGFW_window_checkEvent(win, &event)) {
            if (event.type == RGFW_quit) {
                break;
            }

            if (event.type == RGFW_windowResized) {
                config.window_width = win->w;
                config.window_height = win->h;
                renderer_set_viewport(renderer, win->w, win->h);
                printf("Window resized: %dx%d\n", win->w, win->h);
            }

            if (event.type == RGFW_keyPressed) {
                handle_input(win, (RGFW_keyEvent*)&event, &config, renderer, ps, &camera);
            }
        }
        
        particle_system_update(ps, &config, &camera, dt);

        renderer_update_particles(renderer, ps);
        renderer_draw(renderer, ps, &config, &camera);
        RGFW_window_swapBuffers_OpenGL(win);
    }
    
    // Cleanup
    particle_system_destroy(ps);
    renderer_destroy(renderer);
    RGFW_window_close(win);
    
    // Save configuration
    config_save_to_file(&config, "config.ini");

    return 0;
}