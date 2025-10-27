#include "../field_common.h"

// Field 3: Crystalline Nebula
FIELD_IMPL(field_3) {
    vec2 v;
    
    float r = sqrtf(p.x * p.x + p.y * p.y);
    float theta = atan2f(p.y, p.x);
    
    // Pre-compute common values
    float r_inv = 1.0f / (r + 0.5f);
    float r_scaled = r * 4.0f;
    
    // 1. Simplified hexagonal (2 samples instead of 6)
    float angle1 = 0.0f;
    float angle2 = PI / 3.0f;
    float proj1 = p.x * cosf(angle1) + p.y * sinf(angle1);
    float proj2 = p.x * cosf(angle2) + p.y * sinf(angle2);
    float hex = (cosf(proj1 * 4.0f) + cosf(proj2 * 4.0f)) * 0.1f;
    
    // 2. Simplified interference (2 harmonics instead of 4)
    float interference = (sinf(r * 2.5f + theta) + sinf(r * 5.0f + theta * 2.0f) * 0.5f) * 0.1f;
    
    // 3. Organic flow
    float flow_x = sinf(p.y * 2.0f + cosf(p.x * 1.5f)) * 0.6f;
    float flow_y = cosf(p.x * 2.0f + sinf(p.y * 1.5f)) * 0.6f;
    
    // 4. Radial breathing
    float breath = sinf(r * 3.0f) * expf(-r * 0.3f) * 0.4f;
    
    // 5. Tangential swirl (pre-computed r_inv)
    float swirl_strength = (1.0f + sinf(r_scaled - theta * 8.0f)) * 0.5f;
    float swirl_x = -p.y * swirl_strength * r_inv;
    float swirl_y = p.x * swirl_strength * r_inv;
    
    // 6. Simplified fractal (1 iteration instead of 3)
    float px = fabsf(p.x) - 0.5f;
    float py = fabsf(p.y) - 0.5f;
    float fractal = sinf(px * 2.0f + py) * 0.1f;
    
    // Combine (simplified)
    float cos_theta = cosf(theta * 5.0f);
    float sin_theta = sinf(theta * 5.0f);
    
    v.x = swirl_x 
          + flow_x * (1.0f + hex)
          + cos_theta * breath
          + interference * sin_theta
          + fractal * cosf(r * 2.0f);
    
    v.y = swirl_y 
          + flow_y * (1.0f + hex)
          + sin_theta * breath
          + interference * cos_theta
          + fractal * sinf(r * 2.0f);
    
    // Simplified damping
    v.x *= scale;
    v.y *= scale;
    return v;
}

REGISTER_FIELD(2, field_3, "Crystalline Nebula");