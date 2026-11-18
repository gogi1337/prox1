#version 330 core
layout(location = 0) in vec2 a_pos;    // pixels, origin top-left
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;

uniform vec2 u_screen;                 // window size in pixels

out vec2 v_uv;
out vec4 v_color;

void main() {
    vec2 ndc = vec2(a_pos.x / u_screen.x * 2.0 - 1.0, 1.0 - a_pos.y / u_screen.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
    v_color = a_color;
}
