#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Configuration structure
typedef struct {
    // Window settings
    int window_width;
    int window_height;
    
    // Particle settings
    int particle_count;
    float particle_lifetime;
    
    // Vector field settings
    int vector_field_num;
    float field_scale;
    
    // Integration settings
    float integration_step;
    
    // Simulation settings
    float simulation_speed;
    bool paused;

} Config;

// Function declarations
Config config_create_default();
bool config_load_from_file(Config* config, const char* filename);
void config_print(const Config* config);

#endif // CONFIG_H
