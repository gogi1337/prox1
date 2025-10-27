#include "../field_common.h"

// Field 7: Double Gyre
FIELD_IMPL(field_7) {
    vec2 v;
    const float A = 0.1f;
    const float epsilon = 0.25f;
    const float omega = TWO_PI / 10.0f;
    
    float time_param = p.x + p.y * 0.5f;
    float a = epsilon * sinf(omega * time_param);
    float b = 1.0f - 2.0f * epsilon * sinf(omega * time_param);
    float f = a * p.x * p.x + b * p.x;
    
    v.x = -PI * A * sinf(PI * f) * cosf(PI * p.y);
    v.y = PI * A * cosf(PI * f) * sinf(PI * p.y) * (2.0f * a * p.x + b);
    
    v.x += sinf(p.y * 3.0f) * 0.1f;
    v.y += cosf(p.x * 3.0f) * 0.1f;
    
    v.x *= scale;
    v.y *= scale;
    return v;
}

REGISTER_FIELD(6, field_7, "Double Gyre");