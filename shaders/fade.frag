#version 330 core
uniform vec4 u_fade_color;
out vec4 FragColor;

void main() {
    FragColor = u_fade_color;
}