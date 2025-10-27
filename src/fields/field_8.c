#include "../field_common.h"

// Field 8: Galaxy Spiral
FIELD_IMPL(field_8) {
    vec2 v;
    float r = safe_length(p);
    float theta = atan2f(p.y, p.x);
    
    float spiral = 0.5f / (r + 0.1f);
    float vr = -0.2f * r;
    float vtheta = spiral;
    
    v.x = (vr * cosf(theta) - vtheta * sinf(theta)) * scale;
    v.y = (vr * sinf(theta) + vtheta * cosf(theta)) * scale;
    
    return v;
}

REGISTER_FIELD(7, field_8, "Galaxy Spiral");