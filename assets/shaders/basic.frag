#version 460 core

layout (location = 0) in vec2 TexCoords;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 FragPos;
out vec4 FragColor;

uniform vec3 sunPosition;
uniform vec3 eye;
uniform sampler2D sampler;

void main() {
  float ambientStrength = 0.1;
  vec3 lightColor = vec3(1.0, 1.0, 1.0);
  vec3 ambient = 0.1 * lightColor;

  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(sunPosition - FragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  float specularStrength = 0.5;
  vec3 viewDir = normalize(eye - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  FragColor = vec4((ambient + diffuse + specular), 1.0) * texture(sampler, TexCoords);
}
