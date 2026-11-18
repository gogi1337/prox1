#define _POSIX_C_SOURCE 200809L  // clock_gettime under -std=c99
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
#include "hud.h"

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Wall-clock delta time in seconds (clock() would measure CPU time, not real time)
float get_delta_time() {
    static double last = 0.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double now = (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;

    if (last == 0.0) {
        last = now;
        return 0.016f; // Default to ~60 FPS
    }

    float dt = (float)(now - last);
    last = now;

    // After a stall (window drag, breakpoint) don't make particles jump
    return dt > 0.05f ? 0.05f : dt;
}

static bool show_hud = true;

// Info panel (top-left), cursor crosshair and a key hint line (bottom-left)
static void draw_hud(Hud* hud, const Config* config, const Camera* cam, RGFW_window* win, float fps) {
    static const float panel[4] = {0.0f, 0.0f, 0.0f, 0.60f};
    static const float label[4] = {0.55f, 0.75f, 0.90f, 1.0f};
    static const float value[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    static const float warn[4]  = {1.0f, 0.70f, 0.30f, 1.0f};
    static const float cross[4] = {1.0f, 1.0f, 1.0f, 0.55f};
    const float pad = 8.0f, line = 20.0f;

    int w = win->w, h = win->h;
    int mx = 0, my = 0;
    RGFW_window_getMouse(win, &mx, &my);
    bool inside = mx >= 0 && my >= 0 && mx < w && my < h;

    float left, right, bottom, top;
    camera_get_view_bounds(cam, &left, &right, &bottom, &top);

    char cursor[64] = "--", vec[64] = "--";
    if (inside) {
        float wx, wy;
        camera_screen_to_world(cam, (float)mx, (float)my, w, h, &wx, &wy);
        snprintf(cursor, sizeof(cursor), "x %+.3f  y %+.3f", wx, wy);
        VectorFieldFunc field = vector_field_get(config->vector_field_num);
        if (field) {
            vec2 v = field((vec2){wx, wy}, config->field_scale);
            snprintf(vec, sizeof(vec), "%+.2f %+.2f |v| %.2f", v.x, v.y, sqrtf(v.x * v.x + v.y * v.y));
        }
    }

    char rows[7][64];
    snprintf(rows[0], 64, "%d: %s", config->vector_field_num + 1, vector_field_get_name(config->vector_field_num));
    snprintf(rows[1], 64, "%s", cursor);
    snprintf(rows[2], 64, "%s", vec);
    snprintf(rows[3], 64, "x %+.2f  y %+.2f  zoom %.2fx", cam->x, cam->y, cam->zoom);
    snprintf(rows[4], 64, "x %+.2f..%+.2f  y %+.2f..%+.2f", left, right, bottom, top);
    snprintf(rows[5], 64, "%d  %.0f fps", config->particle_count, fps);
    snprintf(rows[6], 64, "x%.2f  step %.4f", config->simulation_speed, config->integration_step);
    static const char* names[7] = {"Field    ", "Cursor   ", "Vector   ", "Camera   ", "View     ", "Particles", "Speed    "};

    hud_rect(hud, 6, 6, 9 * 46 + 2 * pad, 7 * line + 2 * pad - 2, panel);
    for (int i = 0; i < 7; i++) {
        float y = 6 + pad + i * line;
        hud_text(hud, 6 + pad, y, label, "%s", names[i]);
        hud_text(hud, 6 + pad + 9 * 10, y, value, "%s", rows[i]);
    }
    if (config->paused) hud_text(hud, 6 + pad + 9 * 36, 6 + pad + 6 * line, warn, "PAUSED");

    if (inside) {
        hud_rect(hud, (float)mx - 8, (float)my, 17, 1, cross);
        hud_rect(hud, (float)mx, (float)my - 8, 1, 17, cross);
    }

    const char* help = "H hud  SPACE pause  R reset  C camera  +/- zoom  WASD pan  1-9,0 field";
    hud_rect(hud, 6, h - 6 - 26, 9 * (float)strlen(help) + 2 * pad, 26, panel);
    hud_text(hud, 6 + pad, h - 6 - 22, label, "%s", help);
}

// Handle keyboard input
void handle_input(RGFW_window* win, RGFW_keyEvent* event, Config* config, Renderer* renderer, ParticleSystem* ps, Camera* camera) {
    switch (event->value) {
        case RGFW_space:
            // Toggle pause
            config->paused = !config->paused;
            printf("Simulation %s\n", config->paused ? "paused" : "resumed");
            break;
            
        case RGFW_h:
            show_hud = !show_hud;
            break;

        case RGFW_r:
            // Reset particles
            particle_system_redistribute_grid(ps, config, camera);
            renderer_request_clear(renderer);
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
        case RGFW_0:  // key 0 is field 10
            config->vector_field_num = (event->value == RGFW_0) ? 9 : event->value - RGFW_1;
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
            particle_system_redistribute(ps, config, camera);
            renderer_request_clear(renderer);
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
    camera.aspect = (float)win->w / (float)win->h;  // the window manager may not give the requested size
    
    Renderer* renderer = renderer_create();
    if (!renderer_init(renderer, win->w, win->h)) {
        printf("Error: Failed to initialize renderer\n");
        RGFW_window_close(win);
        return 1;
    }
    
    Hud hud = {0};
    bool hud_ok = hud_init(&hud);
    if (!hud_ok) printf("Warning: HUD disabled (text shader failed)\n");
    float fps = 60.0f;

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
    printf("H       - Show/hide HUD\n");
    printf("ESC     - Exit\n");
    
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        float dt = get_delta_time();
        if (dt > 0.0f) fps += (1.0f / dt - fps) * 0.05f;

        RGFW_event event;
        while (RGFW_window_checkEvent(win, &event)) {
            if (event.type == RGFW_quit) {
                break;
            }

            if (event.type == RGFW_windowResized) {
                config.window_width = win->w;
                config.window_height = win->h;
                renderer_set_viewport(renderer, win->w, win->h);
                if (win->h > 0) camera.aspect = (float)win->w / (float)win->h;
                printf("Window resized: %dx%d\n", win->w, win->h);
            }

            if (event.type == RGFW_keyPressed) {
                handle_input(win, (RGFW_keyEvent*)&event, &config, renderer, ps, &camera);
            }
        }
        
        particle_system_update(ps, &config, &camera, dt);

        renderer_update_particles(renderer, ps);
        renderer_draw(renderer, ps, &camera);

        hud_begin(&hud);
        if (hud_ok && show_hud) draw_hud(&hud, &config, &camera, win, fps);
        hud_draw(&hud, win->w, win->h);
        RGFW_window_swapBuffers_OpenGL(win);
    }
    
    // Cleanup
    hud_destroy(&hud);
    particle_system_destroy(ps);
    renderer_destroy(renderer);
    RGFW_window_close(win);

    return 0;
}