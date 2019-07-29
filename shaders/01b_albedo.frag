#version 330 core

// output variable for the fragment shader. Usually, it is the final color of the fragment
out vec4 color;
uniform sampler2D albedoTexture;
in vec2 tC;


void main()
{   
    color = texture(albedoTexture, tC);
}