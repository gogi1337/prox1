#include "../field_common.h"

// Field 1: Lorenz field
FIELD_IMPL(field_1) {
    vec2 v;
    float sigma = 10.0f;
    float rho = 28.0f;
    v.x = sigma * (p.y - p.x) * 0.05f * scale;
    v.y = (p.x * (rho - p.x * p.x - p.y * p.y) - p.y) * 0.05f * scale;
    return v;
}

// Auto-register this field at startup
REGISTER_FIELD(0, field_1, "Lorenz Field");