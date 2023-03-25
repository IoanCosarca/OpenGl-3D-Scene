#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int activateFog;

void main()
{
    color = texture(skybox, textureCoordinates);
}
