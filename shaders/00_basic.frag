#version 330 core

out vec4 color;

uniform sampler2D textureAlbedoBuffer;
uniform sampler2D textureWorldPosBuffer; //world position
uniform sampler2D textureNormalBuffer;
uniform sampler2D crackedTexture;

uniform mat4 decalMatrixWorldPos;

in vec2 pos;

void main()
{
    //get worldposition
    vec4 worldPosition = texture(textureWorldPosBuffer, pos * 0.5 + 0.5); 
    vec4 objPos = worldPosition * inverse(decalMatrixWorldPos);
    vec4 absObjPos = abs(objPos);
    if(absObjPos.x > 1 || absObjPos.y > 1 || absObjPos.z > 1 || objPos.xyz == vec3(0,0,0)){
        color = texture(textureAlbedoBuffer, pos * 0.5 + 0.5);
    }
    //i am inside the cube decal
    else {
        vec4 decalColor = texture(crackedTexture, objPos.xy * 0.5 + 0.5);
        //manage pixels transparency of the decal
        color = mix(decalColor, texture(textureAlbedoBuffer, pos * 0.5 + 0.5), 1 - decalColor.a);
    }
}