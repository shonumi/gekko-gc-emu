#version 150

uniform sampler2D texture0;

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;

void main() {
    //fragmentColor = texture(texture0, vertexTexCoord0.st).rgba;
    fragmentColor = texture2D(texture0, vertexTexCoord0) * vertexColor;
};