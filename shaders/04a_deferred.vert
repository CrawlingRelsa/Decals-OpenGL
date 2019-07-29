#version 330 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

//textureCoord
out vec2 tC;
out vec3 worldPos;
out vec3 _normal;


void main()
{
    _normal = mat3(transpose(inverse(modelMatrix))) * normal;
    tC = textCoord;
    worldPos = (modelMatrix * vec4(position, 1.0f)).xyz;
    // transformations are applied to each vertex 
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}
