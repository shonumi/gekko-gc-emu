#version 150

uniform sampler2D texture0;

in vec4 gsColor;
in vec2 gsTexCoord0;
out vec4 fragmentColor;
vec4 cc_one = vec4(1.0, 1.0, 1.0, 1.0);
void main() {
    fragmentColor = texture2D(texture0, gsTexCoord0) * gsColor;
}; 