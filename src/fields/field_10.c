#include "../field_common.h"

// Field 10: Atom
// Electrons orbit a nucleus on concentric shells. Each shell pulls particles
// onto a lobed orbit (2, 4, 6, 8, 10 lobes) and neighbouring shells spin in
// opposite directions. The centre is empty, like a nucleus.
FIELD_IMPL(field_10) {
    static const float shell_r[5] = {0.22f, 0.50f, 0.82f, 1.20f, 1.70f};

    float r = sqrtf(p.x * p.x + p.y * p.y + 1e-8f);
    float ux = p.x / r;  // unit vector pointing away from the nucleus
    float uy = p.y / r;

    // Nearest shell decides where this particle is headed
    int n = 0;
    float best = fabsf(r - shell_r[0]);
    for (int i = 1; i < 5; i++) {
        float d = fabsf(r - shell_r[i]);
        if (d < best) { best = d; n = i; }
    }

    // Lobed orbit: shell radius wobbles around the circle
    float theta = atan2f(p.y, p.x);
    float target = shell_r[n] * (1.0f + 0.10f * cosf(2.0f * (n + 1) * theta));
    float d = fminf(fmaxf(r - target, -0.4f), 0.4f);

    float v_r = -4.0f * d;                       // pull onto the orbit
    float v_t = 0.6f / sqrtf(shell_r[n]);        // inner shells are faster
    if (n & 1) v_t = -v_t;

    vec2 v;
    v.x = (ux * v_r - uy * v_t) * scale;
    v.y = (uy * v_r + ux * v_t) * scale;
    return v;
}

REGISTER_FIELD(9, field_10, "Atom");
