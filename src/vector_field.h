#ifndef VECTOR_FIELD_H
#define VECTOR_FIELD_H

#include "config.h"

// 2D vector structure
typedef struct {
    float x;
    float y;
} vec2;

// Function pointer type for vector fields
typedef vec2 (*VectorFieldFunc)(vec2 p, float scale);

// Vector utility functions
vec2 vec2_create(float x, float y);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, float s);
float vec2_length(vec2 v);
float vec2_dot(vec2 a, vec2 b);
vec2 vec2_normalize(vec2 v);

// Individual vector field functions
vec2 field_vortex(vec2 p, float scale);
vec2 field_saddle(vec2 p, float scale);
vec2 field_source(vec2 p, float scale);
vec2 field_sink(vec2 p, float scale);
vec2 field_wave(vec2 p, float scale);
vec2 field_lorenz(vec2 p, float scale);
vec2 field_hopf(vec2 p, float scale);
vec2 field_galaxy(vec2 p, float scale);

// Get vector field function by type
VectorFieldFunc get_vector_field(int field_type);

// Main velocity function
vec2 get_velocity(vec2 p, int field_type, float scale);

// Main evaluation function (used by particle system)
vec2 vector_field_evaluate(vec2 p, const Config* config);

#endif // VECTOR_FIELD_H
