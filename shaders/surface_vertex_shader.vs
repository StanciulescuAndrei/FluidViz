#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 aNormal; 

uniform mat4 rotMatrix;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out vec3 vertexPos;
out vec3 Normal;

void main()
{
	
    gl_Position = projection * view * rotMatrix * vec4(aPos, 1.0);
	vec4 transNormal = mat4(transpose(inverse(rotMatrix))) * vec4(aNormal, 1.0);
    ourColor = color;
	vertexPos = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
	Normal = vec3(transNormal);
}
