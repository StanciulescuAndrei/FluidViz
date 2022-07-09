#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;

uniform mat4 rotMatrix;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;

void main()
{
	ourColor = color;
    gl_Position = projection * view * rotMatrix * vec4(aPos, 1.0);
}
