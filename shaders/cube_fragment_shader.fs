#version 330 core
out vec4 FragColor;

uniform vec3 lightPos;  
uniform vec3 lightColor;  
uniform vec3 viewPos;

in vec3 ourColor;
in vec3 vertexPos;

void main()
{
    float ambientStrength = 1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = (ambient) * ourColor;
    FragColor = vec4(result, 1.0);
}

