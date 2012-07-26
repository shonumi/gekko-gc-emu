#version 150

// Textures
uniform int tex_enable[8];
uniform sampler2D texture0;

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;

void main() {
    if (tex_enable[0] == 1) {
        fragmentColor = texture2D(texture0, vertexTexCoord0) * vertexColor;
    } else {
        fragmentColor = vertexColor;
    }
}