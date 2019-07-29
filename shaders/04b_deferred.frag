#version 330 core

layout (location = 0) out vec3 gWorldPos;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec3 gNormal;


uniform float near;
uniform float far;


uniform sampler2D albedoTexture;
in vec2 tC;
in vec3 worldPos;
in vec3 _normal;

float LinearizeDepth();

void main()
{   
    gWorldPos = worldPos;
    vec2 fixedTexture = tC.xy * vec2(1.0, -1.0);
    gAlbedo = texture(albedoTexture, tC).xyz; 
    gNormal = normalize(_normal);
    
}


float LinearizeDepth() 
{
    float z = gl_FragCoord.z * 2.0 - 1.0; // back to NDC 
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;	
}