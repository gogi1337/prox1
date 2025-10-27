#include "../field_common.h"

// Field 5: Radial Wave Vortex
FIELD_IMPL(field_5) {
    vec2 v;
    float r = safe_length(p);
    v.x = (-p.y + sinf(r * 2.0f) * 0.3f) * scale;
    v.y = (p.x + cosf(r * 2.0f) * 0.3f) * scale;
    return v;
}

REGISTER_FIELD(4, field_5, "Radial Wave Vortex");
