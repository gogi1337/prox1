#include "../field_common.h"

// Field 6: Kármán Vortex Street
FIELD_IMPL(field_6) {
    vec2 v;
    const float frequency = 2.0f;
    const float strength = 1.0f;
    
    float vortex1_y = sinf(p.x * frequency) * 0.5f;
    float vortex2_y = sinf(p.x * frequency + PI) * 0.5f;
    
    vec2 v1_center = {0.0f, vortex1_y};
    vec2 v2_center = {0.0f, vortex2_y};
    
    float dist1 = safe_distance(p, v1_center, 0.1f);
    float dist2 = safe_distance(p, v2_center, 0.1f);
    
    float v1_x = -(p.y - vortex1_y) / dist1;
    float v1_y = p.x / dist1;
    
    float v2_x = (p.y - vortex2_y) / dist2;
    float v2_y = -p.x / dist2;
    
    v.x = strength * (v1_x + v2_x + 0.5f) * scale;
    v.y = strength * (v1_y + v2_y) * scale;
    
    return v;
}

REGISTER_FIELD(5, field_6, "Kármán Vortex Street");
