#include "../field_common.h"

// Field 4: Turbulent Saddle
FIELD_IMPL(field_4) {
    vec2 v;
    v.x = (sinf(p.x * p.y * 0.5f) * 3.0f - cosf(p.y * 2.0f)) * scale;
    v.y = (cosf(p.x * p.y * 0.5f) * 3.0f + sinf(p.x * 2.0f)) * scale;
    return v;
}

REGISTER_FIELD(3, field_4, "Turbulent Saddle");
