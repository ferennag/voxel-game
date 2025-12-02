#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;

layout (location = 0) out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  gl_Position = projection * view * model * vec4(inPos, 1.0);
  TexCoords = inTexCoords;
}

