#ifndef FIELD_COMMON_H
#define FIELD_COMMON_H

#include "vector_field.h"

#include <math.h>

// Constants
#define PI 3.14159265f
#define TWO_PI 6.28318531f

// Field metadata structure
typedef struct {
    const char* name;
    VectorFieldFunc func;
    int index;
} FieldMetadata;

// Macro to register a field automatically
#define REGISTER_FIELD(idx, func_name, display_name) \
    __attribute__((constructor)) \
    static void register_##func_name() { \
        extern void vector_field_register(int, VectorFieldFunc, const char*); \
        vector_field_register(idx, func_name, display_name); \
    }

// Helper macro for field implementation
#define FIELD_IMPL(name) vec2 name(vec2 p, float scale)

// Common helper functions
static inline float safe_length(vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

static inline float safe_distance(vec2 a, vec2 b, float epsilon) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy) + epsilon;
}

#endif // FIELD_COMMON_H