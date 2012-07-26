#version 150

// Textures
uniform int tex_enable[8];
uniform sampler2D texture0;

in vec4 gsColor;
in vec2 gsTexCoord0;
out vec4 fragmentColor;

void main() {
    if (tex_enable[0] == 1) {
        fragmentColor = texture2D(texture0, gsTexCoord0) * gsColor;
    } else {
        fragmentColor = gsColor;
    }
}; 