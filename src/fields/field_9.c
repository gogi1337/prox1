#include "../field_common.h"

// Field 9: Van der Pol Oscillator
FIELD_IMPL(field_9) {
    vec2 v;
    const float mu = 2.0f;
    v.x = p.y * scale;
    v.y = (mu * (1.0f - p.x * p.x) * p.y - p.x) * scale;
    return v;
}

REGISTER_FIELD(8, field_9, "Van der Pol Oscillator");
