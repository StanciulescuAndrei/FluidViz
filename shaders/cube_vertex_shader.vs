#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 rotMatrix;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out vec3 vertexPos;

void main()
{
	
    gl_Position = projection * view * rotMatrix * vec4(aPos, 1.0);
    ourColor = vec3(1.0, 0.1, 0.1);
}
