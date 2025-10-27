#include "../field_common.h"

// Field 3: Circular Waves
FIELD_IMPL(field_3) {
    vec2 v;
    v.x = sinf(p.y * 3.0f) * cosf(p.x * 2.0f) * scale;
    v.y = cosf(p.x * 3.0f) * sinf(p.y * 2.0f) * scale;
    return v;
}

REGISTER_FIELD(2, field_3, "Circular Waves");