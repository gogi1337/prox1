#include "vector_field.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

// Vector utility functions
vec2 vec2_create(float x, float y) {
    vec2 v = {x, y};
    return v;
}

vec2 vec2_add(vec2 a, vec2 b) {
    return vec2_create(a.x + b.x, a.y + b.y);
}

vec2 vec2_sub(vec2 a, vec2 b) {
    return vec2_create(a.x - b.x, a.y - b.y);
}

vec2 vec2_scale(vec2 v, float s) {
    return vec2_create(v.x * s, v.y * s);
}

float vec2_length(vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vec2_dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

vec2 vec2_normalize(vec2 v) {
    float len = vec2_length(v);
    if (len > 0.0001f) {
        return vec2_scale(v, 1.0f / len);
    }
    return vec2_create(0.0f, 0.0f);
}

// Vortex field: circular rotation around origin
vec2 field_vortex(vec2 p, float scale) {
    vec2 v;
    v.x = -p.y * scale;
    v.y = p.x * scale;
    return v;
}

// Saddle point field: hyperbolic flow
vec2 field_saddle(vec2 p, float scale) {
    vec2 v;
    v.x = p.x * scale;
    v.y = -p.y * scale;
    return v;
}

// Source field: radial outward flow
vec2 field_source(vec2 p, float scale) {
    vec2 v;
    float len = vec2_length(p);
    if (len > 0.0001f) {
        v.x = (p.x / len) * scale;
        v.y = (p.y / len) * scale;
    } else {
        v.x = 0.0f;
        v.y = 0.0f;
    }
    return v;
}

// Sink field: radial inward flow
vec2 field_sink(vec2 p, float scale) {
    vec2 v = field_source(p, scale);
    v.x = -v.x;
    v.y = -v.y;
    return v;
}

// Wave field: sinusoidal patterns (your example)
vec2 field_wave(vec2 p, float scale) {
    vec2 v;
    float len = vec2_length(p);
    
    v.x = p.y * scale;
    v.y = (cosf(len) + sinf(len)) * scale;
    
    return v;
}

// Get vector field function by type
VectorFieldFunc get_vector_field(int field_type) {
    switch (field_type) {
        case 0: return field_vortex;
        case 1: return field_saddle;
        case 2: return field_source;
        case 3: return field_sink;
        case 4: return field_wave;
        default: return field_vortex;
    }
}

// Main velocity function
vec2 get_velocity(vec2 p, int field_type, float scale) {
    VectorFieldFunc func = get_vector_field(field_type);
    return func(p, scale);
}

vec2 vector_field_evaluate(vec2 p, const Config* config) {
    vec2 v = {0.0f, 0.0f};
    float scale = config->field_scale;
    
    switch (config->vector_field_type) {
        case FIELD_VORTEX:
            // Swirling vortex
            v.x = -p.y + sinf(p.x * 0.5f) * 0.5f;
            v.y = p.x + cosf(p.y * 0.5f) * 0.5f;
            break;
            
        case FIELD_SADDLE:
            // Wavy flow
            v.x = sinf(5.0f * p.y + p.x);
            v.y = cosf(5.0f * p.x - p.y);
            break;
            
        case FIELD_SOURCE:
            // Circular waves
            v.x = sinf(p.y * 3.0f) * cosf(p.x * 2.0f);
            v.y = cosf(p.x * 3.0f) * sinf(p.y * 2.0f);
            break;
            
        case FIELD_SINK:
            // Turbulent flow with high speed zones
            v.x = sinf(p.x * p.y * 0.5f) * 3.0f - cosf(p.y * 2.0f);
            v.y = cosf(p.x * p.y * 0.5f) * 3.0f + sinf(p.x * 2.0f);
            break;
            
        case FIELD_WAVE:
            // Beautiful wave pattern
            v.x = -p.y + sinf(sqrtf(p.x * p.x + p.y * p.y) * 2.0f) * 0.3f;
            v.y = p.x + cosf(sqrtf(p.x * p.x + p.y * p.y) * 2.0f) * 0.3f;
            break;
    }
    
    v.x *= scale;
    v.y *= scale;
    
    return v;
}