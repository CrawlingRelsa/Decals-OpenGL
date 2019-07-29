
#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec2 pos;

void main()
{
    pos = position.xy;
    gl_Position = vec4(position, 1.0f);
}