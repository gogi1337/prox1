#include "vector_field.h"

#include <stdio.h>
#include <string.h>

#define MAX_FIELDS 32

typedef struct {
    VectorFieldFunc func;
    char name[64];
    int registered;
} FieldEntry;

static FieldEntry field_registry[MAX_FIELDS] = {0};

// Called by REGISTER_FIELD from each field file at startup
void vector_field_register(int index, VectorFieldFunc func, const char* name) {
    if (index < 0 || index >= MAX_FIELDS) {
        fprintf(stderr, "Warning: Field index %d out of range (max: %d)\n", index, MAX_FIELDS - 1);
        return;
    }
    if (field_registry[index].registered) {
        fprintf(stderr, "Warning: Field index %d already taken by '%s', ignoring '%s'\n",
                index, field_registry[index].name, name);
        return;
    }
    field_registry[index].func = func;
    strncpy(field_registry[index].name, name, sizeof(field_registry[index].name) - 1);
    field_registry[index].registered = 1;
    printf("Registered field %d: %s\n", index, name);
}

VectorFieldFunc vector_field_get(int index) {
    if (index >= 0 && index < MAX_FIELDS && field_registry[index].registered) {
        return field_registry[index].func;
    }

    static int warned_index = -1;  // called every frame: warn once per bad index
    for (int i = 0; i < MAX_FIELDS; i++) {
        if (field_registry[i].registered) {
            if (warned_index != index) {
                fprintf(stderr, "Warning: Vector field %d not registered, using %d\n", index, i);
                warned_index = index;
            }
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
    return "(none)";
}
