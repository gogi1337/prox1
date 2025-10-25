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

// Wave field: sinusoidal patterns
vec2 field_wave(vec2 p, float scale) {
    vec2 v;
    float len = vec2_length(p);
    
    v.x = p.y * scale;
    v.y = (cosf(len) + sinf(len)) * scale;
    
    return v;
}

// Lorenz field: chaotic attractor
vec2 field_lorenz(vec2 p, float scale) {
    vec2 v;
    float sigma = 10.0f;
    float rho = 28.0f;
    v.x = sigma * (p.y - p.x) * 0.05f * scale;
    v.y = (p.x * (rho - p.x * p.x - p.y * p.y) - p.y) * 0.05f * scale;
    return v;
}

// Hopf field: spiral bifurcation
vec2 field_hopf(vec2 p, float scale) {
    vec2 v;
    float r_squared = p.x * p.x + p.y * p.y;
    float mu = 1.0f - r_squared;
    v.x = (mu * p.x - p.y) * scale;
    v.y = (p.x + mu * p.y) * scale;
    return v;
}

// Galaxy field: spiral with radial component
vec2 field_galaxy(vec2 p, float scale) {
    vec2 v;
    float r = sqrtf(p.x * p.x + p.y * p.y);
    float theta = atan2f(p.y, p.x);
    float spiral = 0.5f / (r + 0.1f);
    
    float vr = -0.2f * r;
    float vtheta = spiral;
    
    v.x = (vr * cosf(theta) - vtheta * sinf(theta)) * scale;
    v.y = (vr * sinf(theta) + vtheta * cosf(theta)) * scale;
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
        case 5: return field_lorenz;
        case 6: return field_hopf;
        case 7: return field_galaxy;
        default: return field_vortex;
    }
}

// Main velocity function
vec2 get_velocity(vec2 p, int field_type, float scale) {
    VectorFieldFunc func = get_vector_field(field_type);
    return func(p, scale);
}

// Main evaluation function with custom implementations
vec2 vector_field_evaluate(vec2 p, const Config* config) {
    vec2 v = {0.0f, 0.0f};
    float scale = config->field_scale;
    
    switch (config->vector_field_type) {
        case FIELD_VORTEX:
            // Swirling vortex with perturbations
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
            {
                float r = sqrtf(p.x * p.x + p.y * p.y);
                v.x = -p.y + sinf(r * 2.0f) * 0.3f;
                v.y = p.x + cosf(r * 2.0f) * 0.3f;
            }
            break;
            
        case FIELD_LORENZ:
            // Lorenz attractor-inspired (chaotic butterfly)
            {
                float sigma = 10.0f;
                float rho = 28.0f;
                v.x = sigma * (p.y - p.x) * 0.05f;
                v.y = (p.x * (rho - p.x * p.x - p.y * p.y) - p.y) * 0.05f;
            }
            break;
            
        case FIELD_HOPF:
            // Hopf bifurcation (spiral collapse/expansion)
            {
                float r_squared = p.x * p.x + p.y * p.y;
                float mu = 1.0f - r_squared;
                v.x = mu * p.x - p.y;
                v.y = p.x + mu * p.y;
            }
            break;
            
        case FIELD_GALAXY:
            // Galaxy spiral with radial component
            {
                float r = sqrtf(p.x * p.x + p.y * p.y);
                float theta = atan2f(p.y, p.x);
                float spiral = 0.5f / (r + 0.1f);
                
                float vr = -0.2f * r;
                float vtheta = spiral;
                
                v.x = vr * cosf(theta) - vtheta * sinf(theta);
                v.y = vr * sinf(theta) + vtheta * cosf(theta);
            }
            break;
            
        default:
            // Fallback to simple vortex
            v.x = -p.y;
            v.y = p.x;
            break;
    }
    
    v.x *= scale;
    v.y *= scale;
    
    return v;
}