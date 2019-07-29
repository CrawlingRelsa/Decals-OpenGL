#version 330 core

out vec4 color;

uniform sampler2D crackedTexture;

in vec2 texCoord;


void main()
{
    //color = vec4(0.0,1.0,0.0,1.0);
    if(texCoord.x == 0 || texCoord.y == 0) color = vec4(texCoord.y,0.0,0.0,1.0);
    else color = texture(crackedTexture, texCoord);
    
}