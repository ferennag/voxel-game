#version 460 core

layout (location = 0) in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D sampler;

void main() {
  FragColor = texture(sampler, TexCoords);
}
