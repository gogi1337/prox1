#define GL_GLEXT_PROTOTYPES
#include "hud.h"
#include "font_atlas.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glext.h>

#define FLOATS_PER_VERTEX 8   // x, y, u, v, r, g, b, a
#define MAX_QUADS 4096
#define ATLAS_W (FONT_CELL_W * FONT_COLS)
#define ATLAS_H (FONT_CELL_H * FONT_ROWS)

bool hud_init(Hud* hud) {
    memset(hud, 0, sizeof(*hud));

    hud->shader = shader_create_program("shaders/text.vert", "shaders/text.frag");
    if (!hud->shader.is_valid) return false;
    hud->screen_loc = glGetUniformLocation(hud->shader.program_id, "u_screen");
    hud->atlas_loc = glGetUniformLocation(hud->shader.program_id, "u_atlas");

    hud->verts = (float*)malloc(sizeof(float) * FLOATS_PER_VERTEX * 6 * MAX_QUADS);
    if (!hud->verts) return false;

    glGenTextures(1, &hud->atlas);
    glBindTexture(GL_TEXTURE_2D, hud->atlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // atlas rows are 144 bytes, not a multiple of 4 in general
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_W, ATLAS_H, 0, GL_RED, GL_UNSIGNED_BYTE, FONT_ATLAS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenVertexArrays(1, &hud->vao);
    glGenBuffers(1, &hud->vbo);
    glBindVertexArray(hud->vao);
    glBindBuffer(GL_ARRAY_BUFFER, hud->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * FLOATS_PER_VERTEX * 6 * MAX_QUADS, NULL, GL_DYNAMIC_DRAW);
    GLsizei stride = FLOATS_PER_VERTEX * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return true;
}

void hud_begin(Hud* hud) {
    hud->vertex_count = 0;
}

static void push_quad(Hud* hud, float x0, float y0, float x1, float y1,
                      float u0, float v0, float u1, float v1, const float c[4]) {
    if (hud->vertex_count + 6 > 6 * MAX_QUADS) return;  // overlay full: drop the rest
    float q[6][4] = {
        {x0, y0, u0, v0}, {x1, y0, u1, v0}, {x1, y1, u1, v1},
        {x0, y0, u0, v0}, {x1, y1, u1, v1}, {x0, y1, u0, v1},
    };
    float* out = hud->verts + (size_t)hud->vertex_count * FLOATS_PER_VERTEX;
    for (int i = 0; i < 6; i++) {
        memcpy(out, q[i], 4 * sizeof(float));
        memcpy(out + 4, c, 4 * sizeof(float));
        out += FLOATS_PER_VERTEX;
    }
    hud->vertex_count += 6;
}

void hud_rect(Hud* hud, float x, float y, float w, float h, const float rgba[4]) {
    int col = FONT_SOLID_CELL % FONT_COLS, row = FONT_SOLID_CELL / FONT_COLS;
    float u = (col * FONT_CELL_W + FONT_CELL_W * 0.5f) / ATLAS_W;
    float v = (row * FONT_CELL_H + FONT_CELL_H * 0.5f) / ATLAS_H;
    push_quad(hud, x, y, x + w, y + h, u, v, u, v, rgba);
}

void hud_text(Hud* hud, float x, float y, const float rgba[4], const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (const char* s = buf; *s; s++) {
        int c = (unsigned char)*s;
        if (c < 32 || c > 126) c = '?';
        if (c != ' ') {
            int idx = c - 32, col = idx % FONT_COLS, row = idx / FONT_COLS;
            float u0 = (float)(col * FONT_CELL_W) / ATLAS_W;
            float v0 = (float)(row * FONT_CELL_H) / ATLAS_H;
            float u1 = (float)((col + 1) * FONT_CELL_W) / ATLAS_W;
            float v1 = (float)((row + 1) * FONT_CELL_H) / ATLAS_H;
            push_quad(hud, x, y, x + FONT_CELL_W, y + FONT_CELL_H, u0, v0, u1, v1, rgba);
        }
        x += FONT_CELL_W;
    }
}

void hud_draw(Hud* hud, int window_w, int window_h) {
    if (hud->vertex_count == 0) return;

    glViewport(0, 0, window_w, window_h);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    glUseProgram(hud->shader.program_id);
    glUniform2f(hud->screen_loc, (float)window_w, (float)window_h);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hud->atlas);
    glUniform1i(hud->atlas_loc, 0);

    glBindVertexArray(hud->vao);
    glBindBuffer(GL_ARRAY_BUFFER, hud->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * FLOATS_PER_VERTEX * hud->vertex_count, hud->verts);
    glDrawArrays(GL_TRIANGLES, 0, hud->vertex_count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void hud_destroy(Hud* hud) {
    if (hud->vao) glDeleteVertexArrays(1, &hud->vao);
    if (hud->vbo) glDeleteBuffers(1, &hud->vbo);
    if (hud->atlas) glDeleteTextures(1, &hud->atlas);
    shader_delete(&hud->shader);
    free(hud->verts);
    memset(hud, 0, sizeof(*hud));
}
