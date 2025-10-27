#include "../field_common.h"

// Field 2: Wavy Hyperbolic Flow
FIELD_IMPL(field_2) {
    vec2 v;
    v.x = sinf(5.0f * p.y + p.x) * scale;
    v.y = cosf(5.0f * p.x - p.y) * scale;
    return v;
}

REGISTER_FIELD(1, field_2, "Wavy Hyperbolic");