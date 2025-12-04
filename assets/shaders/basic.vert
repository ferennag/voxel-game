#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec2 TexCoords;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  gl_Position = projection * view * model * vec4(inPos, 1.0);
  TexCoords = inTexCoords;
  FragPos = vec3(model * vec4(inPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * inNormal;
}

