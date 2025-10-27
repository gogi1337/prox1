#include "../field_common.h"

// Field 4: Hopf field
FIELD_IMPL(field_4) {
    vec2 v;
    float r_squared = p.x * p.x + p.y * p.y;
    float mu = 1.0f - r_squared;
    v.x = (mu * p.x - p.y) * scale;
    v.y = (p.x + mu * p.y) * scale;
    return v;
}

REGISTER_FIELD(3, field_4, "Hopf Field");