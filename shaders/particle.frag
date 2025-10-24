#version 330 core

in vec4 particleColor;
out vec4 FragColor;

void main() {
    // Calculate distance from center
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord) * 2.0;
    
    // Soft glow - no hard edges
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    alpha = pow(alpha, 2.0); // Sharper falloff
    
    // Apply color with glow
    FragColor = vec4(particleColor.rgb, particleColor.a * alpha);
}
