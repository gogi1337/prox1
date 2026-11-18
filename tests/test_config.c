#include "config.h"
#include "vector_field.h"

#include <assert.h>
#include <stdio.h>

static vec2 field_a(vec2 p, float s) { (void)s; return p; }
static vec2 field_b(vec2 p, float s) { (void)s; return p; }

static void write_file(const char* path, const char* text) {
    FILE* f = fopen(path, "wb");
    assert(f);
    fputs(text, f);
    fclose(f);
}

static void test_config(void) {
    const char* path = "build/test_config.ini";

    // CRLF line endings, comments, unknown key, spaces around '='
    write_file(path,
        "# comment\r\n"
        "window_width = 800\r\n"
        "particle_count=1234\r\n"
        "field_scale = 2.50\r\n"
        "unknown_key = 5\r\n");
    Config c = config_create_default();
    assert(config_load_from_file(&c, path));
    assert(c.window_width == 800);
    assert(c.particle_count == 1234);
    assert(c.field_scale == 2.5f);
    assert(c.window_height == 720);  // missing key keeps default

    // Garbage values are clamped to something usable
    write_file(path, "window_width = abc\nparticle_count = -5\nintegration_step = 0\n");
    c = config_create_default();
    assert(config_load_from_file(&c, path));
    assert(c.window_width >= 100 && c.particle_count >= 1 && c.integration_step > 0.0f);

    // Missing file keeps defaults and reports failure
    c = config_create_default();
    assert(!config_load_from_file(&c, "build/does_not_exist.ini"));
    assert(c.window_width == 1280);
}

static void test_registry(void) {
    assert(vector_field_get(0) == NULL);  // nothing registered yet
    vector_field_register(3, field_a, "A");
    vector_field_register(3, field_b, "B");  // duplicate index is rejected
    assert(vector_field_get(3) == field_a);
    assert(vector_field_get(99) == field_a);  // bad index falls back to first field
    assert(vector_field_get(-1) == field_a);
}

int main(void) {
    test_config();
    test_registry();
    printf("all tests passed\n");
    return 0;
}
