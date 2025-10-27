#include "../field_common.h"

// Field 1: Perturbed Vortex
FIELD_IMPL(field_1) {
    vec2 v;
    v.x = (-p.y + sinf(p.x * 0.5f) * 0.5f) * scale;
    v.y = (p.x + cosf(p.y * 0.5f) * 0.5f) * scale;
    return v;
}

// Auto-register this field at startup
REGISTER_FIELD(0, field_1, "Perturbed Vortex");