#include "camera.h"
#include <math.h>

Camera camera_create(void) {
    Camera cam;
    cam.x = 0.0f;
    cam.y = 0.0f;
    cam.zoom = 1.0f;
    cam.min_zoom = 0.1f;
    cam.max_zoom = 10.0f;
    cam.move_speed = 2.0f;
    cam.zoom_speed = 0.1f;
    return cam;
}

void camera_get_view_bounds(const Camera* cam, float* left, float* right, float* bottom, float* top) {
    float half_width = 1.0f / cam->zoom;
    float half_height = 1.0f / cam->zoom;
    
    *left = cam->x - half_width;
    *right = cam->x + half_width;
    *bottom = cam->y - half_height;
    *top = cam->y + half_height;
}

void camera_zoom_in(Camera* cam) {
    cam->zoom *= (1.0f + cam->zoom_speed);
    if (cam->zoom > cam->max_zoom) {
        cam->zoom = cam->max_zoom;
    }
}

void camera_zoom_out(Camera* cam) {
    cam->zoom *= (1.0f - cam->zoom_speed);
    if (cam->zoom < cam->min_zoom) {
        cam->zoom = cam->min_zoom;
    }
}

void camera_pan(Camera* cam, float dx, float dy) {
    cam->x += dx / cam->zoom;
    cam->y += dy / cam->zoom;
}

void camera_reset(Camera* cam) {
    cam->x = 0.0f;
    cam->y = 0.0f;
    cam->zoom = 1.0f;
}

void camera_screen_to_world(const Camera* cam, float screen_x, float screen_y,
                            int window_width, int window_height,
                            float* world_x, float* world_y) {
    // Normalize screen coords to -1 to 1
    float norm_x = (screen_x / window_width) * 2.0f - 1.0f;
    float norm_y = 1.0f - (screen_y / window_height) * 2.0f;
    
    // Apply zoom and camera position
    *world_x = cam->x + norm_x / cam->zoom;
    *world_y = cam->y + norm_y / cam->zoom;
}

void camera_update(Camera* cam, float dt) {
    // Reserved for future velocity-based camera movement
}