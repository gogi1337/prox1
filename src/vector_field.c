#include "vector_field.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

// =============================================================================
// Constants
// =============================================================================

#define MAX_FIELDS 32

// =============================================================================
// Field Registry
// =============================================================================

typedef struct {
    VectorFieldFunc func;
    char name[64];
    bool registered;
} FieldEntry;

static FieldEntry field_registry[MAX_FIELDS] = {0};
static int registered_count = 0;

// =============================================================================
// Vector Utility Functions
// =============================================================================

vec2 vec2_create(float x, float y) {
    vec2 v = {x, y};
    return v;
}

vec2 vec2_add(vec2 a, vec2 b) {
    return vec2_create(a.x + b.x, a.y + b.y);
}

vec2 vec2_sub(vec2 a, vec2 b) {
    return vec2_create(a.x - b.x, a.y - b.y);
}

vec2 vec2_scale(vec2 v, float s) {
    return vec2_create(v.x * s, v.y * s);
}

float vec2_length(vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vec2_dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

vec2 vec2_normalize(vec2 v) {
    float len = vec2_length(v);
    if (len > 0.0001f) {
        return vec2_scale(v, 1.0f / len);
    }
    return vec2_create(0.0f, 0.0f);
}

// =============================================================================
// Registration System (Auto-called by field files)
// =============================================================================

void vector_field_register(int index, VectorFieldFunc func, const char* name) {
    if (index >= 0 && index < MAX_FIELDS) {
        if (!field_registry[index].registered) {
            field_registry[index].func = func;
            strncpy(field_registry[index].name, name, sizeof(field_registry[index].name) - 1);
            field_registry[index].name[sizeof(field_registry[index].name) - 1] = '\0';
            field_registry[index].registered = true;
            registered_count++;
            
            printf("Registered field %d: %s\n", index, name);
        }
    } else {
        fprintf(stderr, "Warning: Field index %d out of range (max: %d)\n", index, MAX_FIELDS);
    }
}

VectorFieldFunc vector_field_get(int index) {
    if (index >= 0 && index < MAX_FIELDS && field_registry[index].registered) {
        return field_registry[index].func;
    }
    
    // Fallback: find first registered field
    for (int i = 0; i < MAX_FIELDS; i++) {
        if (field_registry[i].registered) {
            return field_registry[i].func;
        }
    }
    
    fprintf(stderr, "Error: No vector fields registered!\n");
    return NULL;
}

const char* vector_field_get_name(int index) {
    if (index >= 0 && index < MAX_FIELDS && field_registry[index].registered) {
        return field_registry[index].name;
    }
    return "Unknown";
}

int vector_field_get_count() {
    return registered_count;
}

void vector_field_list_all() {
    for (int i = 0; i < MAX_FIELDS; i++) {
        if (field_registry[i].registered) {
            printf("  [%d] %s\n", i, field_registry[i].name);
        }
    }
    printf("%d registered fields in total\n\n", registered_count);
}

// =============================================================================
// Main Evaluation Functions
// =============================================================================

vec2 get_velocity(vec2 p, int field_type, float scale) {
    VectorFieldFunc func = vector_field_get(field_type);
    if (func) {
        return func(p, scale);
    }
    return vec2_create(0.0f, 0.0f);
}

vec2 vector_field_evaluate(vec2 p, const Config* config) {
    VectorFieldFunc func = vector_field_get(config->vector_field_num);
    if (func) {
        return func(p, config->field_scale);
    }
    return vec2_create(0.0f, 0.0f);
}