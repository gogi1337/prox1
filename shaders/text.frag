#version 330 core
in vec2 v_uv;
in vec4 v_color;

uniform sampler2D u_atlas;

out vec4 FragColor;

void main() {
    FragColor = vec4(v_color.rgb, v_color.a * texture(u_atlas, v_uv).r);
}
