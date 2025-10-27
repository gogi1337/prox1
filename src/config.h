#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Integration method enumeration
typedef enum {
    INTEGRATION_EULER,
    INTEGRATION_RK2,
    INTEGRATION_RK4
} IntegrationMethod;

// Configuration structure
typedef struct {
    // Window settings
    int window_width;
    int window_height;
    
    // Particle settings
    int particle_count;
    float particle_size;
    float particle_lifetime;
    
    // Particle color (RGBA, 0.0 - 1.0)
    float particle_color[4];
    
    // Vector field settings
    int vector_field_num;
    float field_scale;
    
    // Integration settings
    IntegrationMethod integration_method;
    float integration_step;
    
    // Simulation settings
    float simulation_speed;
    bool paused;
    
    // Rendering settings
    float background_color[4];
    int trail_length;
    
} Config;

// Function declarations
Config config_create_default();
bool config_load_from_file(Config* config, const char* filename);
bool config_save_to_file(const Config* config, const char* filename);
void config_print(const Config* config);

#endif // CONFIG_H
