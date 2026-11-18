#ifndef HUD_H
#define HUD_H

#include "shader.h"
#include <stdbool.h>

// Text and rectangle overlay in window pixels (origin top-left).
// Usage per frame: hud_begin, any number of hud_rect / hud_text, hud_draw.
// Shapes are drawn in the order they are added.
typedef struct {
    ShaderProgram shader;
    unsigned int vao, vbo, atlas;
    int screen_loc, atlas_loc;
    float* verts;
    int vertex_count;
} Hud;

bool hud_init(Hud* hud);
void hud_begin(Hud* hud);
void hud_rect(Hud* hud, float x, float y, float w, float h, const float rgba[4]);
void hud_text(Hud* hud, float x, float y, const float rgba[4], const char* fmt, ...);
void hud_draw(Hud* hud, int window_w, int window_h);
void hud_destroy(Hud* hud);

#endif // HUD_H
