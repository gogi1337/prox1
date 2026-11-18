#ifndef VECTOR_FIELD_H
#define VECTOR_FIELD_H

// 2D vector structure
typedef struct {
    float x;
    float y;
} vec2;

// Function pointer type for vector fields
typedef vec2 (*VectorFieldFunc)(vec2 p, float scale);

// Field registration system
void vector_field_register(int index, VectorFieldFunc func, const char* name);
// Returns the field at index, or the first registered field (with a warning) if index is not registered
VectorFieldFunc vector_field_get(int index);

#endif // VECTOR_FIELD_H
