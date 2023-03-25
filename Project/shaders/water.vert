#version 410 core

in vec2 position;

out vec4 clipSpace;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position.x, 0.0f, position.y, 1.0);
	clipSpace = gl_Position;
}