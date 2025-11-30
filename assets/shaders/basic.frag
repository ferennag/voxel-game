#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sampler;

void main() {
  FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
