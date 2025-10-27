#include "vector_field.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

// =============================================================================
// Constants
// =============================================================================

#define PI 3.14159265f
#define TWO_PI 6.28318531f

// =============================================================================
// Vector Utility Functions
// =============================================================================

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

// =============================================================================
// Vector Field Implementations
// =============================================================================

// Field 1: Perturbed Vortex
vec2 field_1(vec2 p, float scale) {
    vec2 v;
    v.x = (-p.y + sinf(p.x * 0.5f) * 0.5f) * scale;
    v.y = (p.x + cosf(p.y * 0.5f) * 0.5f) * scale;
    return v;
}

// Field 2: Wavy Hyperbolic Flow
vec2 field_2(vec2 p, float scale) {
    vec2 v;
    v.x = sinf(5.0f * p.y + p.x) * scale;
    v.y = cosf(5.0f * p.x - p.y) * scale;
    return v;
}

// Field 3: Circular Waves
vec2 field_3(vec2 p, float scale) {
    vec2 v;
    v.x = sinf(p.y * 3.0f) * cosf(p.x * 2.0f) * scale;
    v.y = cosf(p.x * 3.0f) * sinf(p.y * 2.0f) * scale;
    return v;
}

// Field 4: Turbulent Saddle
vec2 field_4(vec2 p, float scale) {
    vec2 v;
    v.x = (sinf(p.x * p.y * 0.5f) * 3.0f - cosf(p.y * 2.0f)) * scale;
    v.y = (cosf(p.x * p.y * 0.5f) * 3.0f + sinf(p.x * 2.0f)) * scale;
    return v;
}

// Field 5: Radial Wave Vortex
vec2 field_5(vec2 p, float scale) {
    vec2 v;
    float r = sqrtf(p.x * p.x + p.y * p.y);
    v.x = (-p.y + sinf(r * 2.0f) * 0.3f) * scale;
    v.y = (p.x + cosf(r * 2.0f) * 0.3f) * scale;
    return v;
}

// Field 6: Kármán Vortex Street (Turbulent Shedding)
vec2 field_6(vec2 p, float scale) {
    vec2 v;
    const float frequency = 2.0f;
    const float strength = 1.0f;
    
    // Oscillating vortex centers
    float vortex1_y = sinf(p.x * frequency) * 0.5f;
    float vortex2_y = sinf(p.x * frequency + PI) * 0.5f;
    
    // Distance to each vortex (with singularity prevention)
    float dist1 = sqrtf(p.x * p.x + (p.y - vortex1_y) * (p.y - vortex1_y)) + 0.1f;
    float dist2 = sqrtf(p.x * p.x + (p.y - vortex2_y) * (p.y - vortex2_y)) + 0.1f;
    
    // Counterclockwise vortex 1
    float v1_x = -(p.y - vortex1_y) / dist1;
    float v1_y = p.x / dist1;
    
    // Clockwise vortex 2
    float v2_x = (p.y - vortex2_y) / dist2;
    float v2_y = -p.x / dist2;
    
    // Combine with base flow
    v.x = strength * (v1_x + v2_x + 0.5f) * scale;
    v.y = strength * (v1_y + v2_y) * scale;
    
    return v;
}

// Field 7: Double Gyre (Chaotic Ocean Flow)
vec2 field_7(vec2 p, float scale) {
    vec2 v;
    const float A = 0.1f;
    const float epsilon = 0.25f;
    const float omega = TWO_PI / 10.0f;
    
    // Position-based time parameter
    float time_param = p.x + p.y * 0.5f;
    float a = epsilon * sinf(omega * time_param);
    float b = 1.0f - 2.0f * epsilon * sinf(omega * time_param);
    float f = a * p.x * p.x + b * p.x;
    
    // Stream function derivatives
    v.x = -PI * A * sinf(PI * f) * cosf(PI * p.y);
    v.y = PI * A * cosf(PI * f) * sinf(PI * p.y) * (2.0f * a * p.x + b);
    
    // Add perturbations for visual complexity
    v.x += sinf(p.y * 3.0f) * 0.1f;
    v.y += cosf(p.x * 3.0f) * 0.1f;
    
    v.x *= scale;
    v.y *= scale;
    return v;
}

// Field 8: Galaxy Spiral
vec2 field_8(vec2 p, float scale) {
    vec2 v;
    float r = sqrtf(p.x * p.x + p.y * p.y);
    float theta = atan2f(p.y, p.x);
    
    // Spiral strength decreases with radius
    float spiral = 0.5f / (r + 0.1f);
    
    // Radial and tangential components
    float vr = -0.2f * r;
    float vtheta = spiral;
    
    // Convert to Cartesian coordinates
    v.x = (vr * cosf(theta) - vtheta * sinf(theta)) * scale;
    v.y = (vr * sinf(theta) + vtheta * cosf(theta)) * scale;
    
    return v;
}

// Field 9: Placeholder for future field
vec2 field_9(vec2 p, float scale) {
    // Van der Pol Oscillator (self-exciting limit cycle)
    vec2 v;
    const float mu = 2.0f;
    v.x = p.y * scale;
    v.y = (mu * (1.0f - p.x * p.x) * p.y - p.x) * scale;
    return v;
}

// =============================================================================
// Field Selection
// =============================================================================

VectorFieldFunc get_vector_field(int field_type) {
    static const VectorFieldFunc field_lookup[] = {
        field_1,  // Perturbed vortex
        field_2,  // Wavy hyperbolic
        field_3,  // Circular waves
        field_4,  // Turbulent saddle
        field_5,  // Radial wave vortex
        field_6,  // Kármán vortex street
        field_7,  // Double gyre
        field_8,  // Galaxy spiral
        field_9   // Van der Pol
    };
    
    const int num_fields = sizeof(field_lookup) / sizeof(field_lookup[0]);
    
    if (field_type >= 0 && field_type < num_fields) {
        return field_lookup[field_type];
    }
    
    return field_1;  // Default fallback
}

// =============================================================================
// Main Evaluation Function
// =============================================================================

vec2 get_velocity(vec2 p, int field_type, float scale) {
    VectorFieldFunc func = get_vector_field(field_type);
    return func(p, scale);
}

vec2 vector_field_evaluate(vec2 p, const Config* config) {
    VectorFieldFunc func = get_vector_field(config->vector_field_type);
    return func(p, config->field_scale);
}