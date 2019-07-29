#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tC;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

out vec2 texCoord;



void main()
{
    texCoord = tC;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}