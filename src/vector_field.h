#ifndef VECTOR_FIELD_H
#define VECTOR_FIELD_H

#include "config.h"

#include <stdbool.h>

// 2D vector structure
typedef struct {
    float x;
    float y;
} vec2;

// Function pointer type for vector fields
typedef vec2 (*VectorFieldFunc)(vec2 p, float scale);

// Vector utility functions
vec2 vec2_create(float x, float y);
vec2 vec2_add(vec2 a, vec2 b);
vec2 vec2_sub(vec2 a, vec2 b);
vec2 vec2_scale(vec2 v, float s);
float vec2_length(vec2 v);
float vec2_dot(vec2 a, vec2 b);
vec2 vec2_normalize(vec2 v);

// Field registration system
void vector_field_register(int index, VectorFieldFunc func, const char* name);
VectorFieldFunc vector_field_get(int index);
const char* vector_field_get_name(int index);
int vector_field_get_count();
void vector_field_list_all();

// Main evaluation functions
vec2 get_velocity(vec2 p, int field_type, float scale);
vec2 vector_field_evaluate(vec2 p, const Config* config);

#endif // VECTOR_FIELD_H