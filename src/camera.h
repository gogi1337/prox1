#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

typedef struct {
    float x;              // Camera position X
    float y;              // Camera position Y
    float zoom;           // Zoom level (1.0 = normal, 2.0 = 2x zoom in)
    float min_zoom;       // Minimum zoom
    float max_zoom;       // Maximum zoom
    float aspect;         // Window width / height
    float zoom_speed;     // Zoom speed
} Camera;

// Create and initialize camera
Camera camera_create(void);

// Get projection matrix values for rendering
void camera_get_view_bounds(const Camera* cam, float* left, float* right, float* bottom, float* top);

// Zoom in/out
void camera_zoom_in(Camera* cam);
void camera_zoom_out(Camera* cam);

// Pan camera
void camera_pan(Camera* cam, float dx, float dy);

// Reset camera to default
void camera_reset(Camera* cam);

#endif // CAMERA_H
