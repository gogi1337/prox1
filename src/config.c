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
    config.particle_size = 2.0f;
    config.particle_lifetime = 10.0f;
    
    // Particle color (white with some transparency)
    config.particle_color[0] = 1.0f;  // R
    config.particle_color[1] = 1.0f;  // G
    config.particle_color[2] = 1.0f;  // B
    config.particle_color[3] = 0.8f;  // A
    
    // Vector field settings
    config.vector_field_num = 1;
    config.field_scale = 1.0f;
    
    // Integration settings
    config.integration_step = 0.01f;
    
    // Simulation settings
    config.simulation_speed = 1.0f;
    config.paused = false;
    
    // Rendering settings
    config.background_color[0] = 0.1f;  // R
    config.background_color[1] = 0.1f;  // G
    config.background_color[2] = 0.1f;  // B
    config.background_color[3] = 1.0f;  // A
    config.trail_length = 0;  // 0 = no trails
    
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
            } else if (strcmp(key_start, "particle_size") == 0) {
                config->particle_size = (float)atof(value_start);
            } else if (strcmp(key_start, "particle_lifetime") == 0) {
                config->particle_lifetime = (float)atof(value_start);
            } else if (strcmp(key_start, "particle_color") == 0) {
                sscanf(value_start, "%f,%f,%f,%f", 
                       &config->particle_color[0],
                       &config->particle_color[1],
                       &config->particle_color[2],
                       &config->particle_color[3]);
            } else if (strcmp(key_start, "vector_field_num") == 0) {
                config->vector_field_num = (int)atoi(value_start);
            } else if (strcmp(key_start, "field_scale") == 0) {
                config->field_scale = (float)atof(value_start);
            } else if (strcmp(key_start, "integration_step") == 0) {
                config->integration_step = (float)atof(value_start);
            } else if (strcmp(key_start, "simulation_speed") == 0) {
                config->simulation_speed = (float)atof(value_start);
            } else if (strcmp(key_start, "trail_length") == 0) {
                config->trail_length = atoi(value_start);
            } else if (strcmp(key_start, "background_color") == 0) {
                sscanf(value_start, "%f,%f,%f,%f",
                       &config->background_color[0],
                       &config->background_color[1],
                       &config->background_color[2],
                       &config->background_color[3]);
            }
        }
    }
    
    fclose(file);
    return true;
}

// Save configuration to file
bool config_save_to_file(const Config* config, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not write to config file '%s'\n", filename);
        return false;
    }
    
    fprintf(file, "# Vector Field Visualizer Configuration\n\n");
    
    fprintf(file, "# Window Settings\n");
    fprintf(file, "window_width = %d\n", config->window_width);
    fprintf(file, "window_height = %d\n\n", config->window_height);
    
    fprintf(file, "# Particle Settings\n");
    fprintf(file, "particle_count = %d\n", config->particle_count);
    fprintf(file, "particle_size = %.2f\n", config->particle_size);
    fprintf(file, "particle_lifetime = %.2f\n", config->particle_lifetime);
    fprintf(file, "particle_color = %.2f,%.2f,%.2f,%.2f\n\n",
            config->particle_color[0], config->particle_color[1],
            config->particle_color[2], config->particle_color[3]);
    
    fprintf(file, "# Vector Field Settings\n");
    fprintf(file, "vector_field_num = %d\n", config->vector_field_num);
    fprintf(file, "field_scale = %.2f\n\n", config->field_scale);
    
    fprintf(file, "# Integration Settings\n");
    fprintf(file, "integration_step = %.4f\n\n", config->integration_step);
    
    fprintf(file, "# Simulation Settings\n");
    fprintf(file, "simulation_speed = %.2f\n\n", config->simulation_speed);
    
    fprintf(file, "# Rendering Settings\n");
    fprintf(file, "trail_length = %d\n", config->trail_length);
    fprintf(file, "background_color = %.2f,%.2f,%.2f,%.2f\n",
            config->background_color[0], config->background_color[1],
            config->background_color[2], config->background_color[3]);
    
    fclose(file);
    return true;
}

// Print configuration (for debugging)
void config_print(const Config* config) {
    printf("=== Configuration ===\n");
    printf("Window: %dx%d\n", config->window_width, config->window_height);
    printf("Particles: %d (size: %.2f, lifetime: %.2f)\n",
           config->particle_count, config->particle_size, config->particle_lifetime);
    printf("Particle Color: (%.2f, %.2f, %.2f, %.2f)\n",
           config->particle_color[0], config->particle_color[1],
           config->particle_color[2], config->particle_color[3]);
    printf("Vector Field: %d (scale: %.2f)\n",
           config->vector_field_num, config->field_scale);
    printf("Integration: step=%.4f\n",
           config->integration_step);
    printf("Simulation Speed: %.2f\n", config->simulation_speed);
    printf("Trail Length: %d\n", config->trail_length);
    printf("Background Color: (%.2f, %.2f, %.2f, %.2f)\n",
           config->background_color[0], config->background_color[1],
           config->background_color[2], config->background_color[3]);
    printf("====================\n");
}
