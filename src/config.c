#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create default configuration
Config config_create_default() {
    Config config;
    
    // Window settings
    config.window_width = 1280;
    config.window_height = 720;
    
    // Particle settings
    config.particle_count = 5000;
    config.particle_lifetime = 10.0f;
    
    // Vector field settings
    config.vector_field_num = 1;
    config.field_scale = 1.0f;
    
    // Integration settings
    config.integration_step = 0.01f;
    
    // Simulation settings
    config.simulation_speed = 1.0f;
    config.paused = false;
    
    return config;
}

// Simple INI-style file parser
bool config_load_from_file(Config* config, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Could not open config file '%s', using defaults\n", filename);
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n') {
            continue;
        }
        
        // Parse key=value pairs
        char key[128], value[128];
        if (sscanf(line, "%127[^=]=%127[^\n]", key, value) == 2) {
            // Trim whitespace from key
            char* key_start = key;
            while (*key_start == ' ' || *key_start == '\t') key_start++;
            char* key_end = key_start + strlen(key_start) - 1;
            while (key_end > key_start && (*key_end == ' ' || *key_end == '\t')) {
                *key_end = '\0';
                key_end--;
            }
            
            // Trim whitespace from value
            char* value_start = value;
            while (*value_start == ' ' || *value_start == '\t') value_start++;
            char* value_end = value_start + strlen(value_start) - 1;
            while (value_end > value_start && (*value_end == ' ' || *value_end == '\t' || *value_end == '\n' || *value_end == '\r')) {
                *value_end = '\0';
                value_end--;
            }
            
            // Match keys and assign values
            if (strcmp(key_start, "window_width") == 0) {
                config->window_width = atoi(value_start);
            } else if (strcmp(key_start, "window_height") == 0) {
                config->window_height = atoi(value_start);
            } else if (strcmp(key_start, "particle_count") == 0) {
                config->particle_count = atoi(value_start);
            } else if (strcmp(key_start, "particle_lifetime") == 0) {
                config->particle_lifetime = (float)atof(value_start);
            } else if (strcmp(key_start, "vector_field_num") == 0) {
                config->vector_field_num = (int)atoi(value_start);
            } else if (strcmp(key_start, "field_scale") == 0) {
                config->field_scale = (float)atof(value_start);
            } else if (strcmp(key_start, "integration_step") == 0) {
                config->integration_step = (float)atof(value_start);
            } else if (strcmp(key_start, "simulation_speed") == 0) {
                config->simulation_speed = (float)atof(value_start);
            }
        }
    }
    
    fclose(file);

    // Typos like "window_width = abc" parse as 0; keep the values usable
    if (config->window_width < 100) config->window_width = 100;
    if (config->window_height < 100) config->window_height = 100;
    if (config->particle_count < 1) config->particle_count = 1;
    if (config->particle_count > 5000000) config->particle_count = 5000000;  // renderer stores 2 vertices each in an int-sized count
    if (config->particle_lifetime < 0.1f) config->particle_lifetime = 0.1f;
    if (config->integration_step < 0.0001f) config->integration_step = 0.0001f;
    return true;
}

// Print configuration (for debugging)
void config_print(const Config* config) {
    printf("=== Configuration ===\n");
    printf("Window: %dx%d\n", config->window_width, config->window_height);
    printf("Particles: %d (lifetime: %.2f)\n",
           config->particle_count, config->particle_lifetime);
    printf("Vector Field: %d (scale: %.2f)\n",
           config->vector_field_num, config->field_scale);
    printf("Integration: step=%.4f\n",
           config->integration_step);
    printf("Simulation Speed: %.2f\n", config->simulation_speed);
    printf("====================\n");
}
