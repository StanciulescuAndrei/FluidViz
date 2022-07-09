#version 330 core
out vec4 FragColor;

uniform vec3 lightPos;  
uniform vec3 lightColor;  
uniform vec3 viewPos;

in vec3 ourColor;
in vec3 vertexPos;
in vec3 Normal;  

void main()
{
    float ambientStrength = 0.2;
    float specularStrength = 0.5;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - vertexPos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - vertexPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    vec3 specular = specularStrength * spec * lightColor;  

    vec3 result = (ambient + diffuse + specular) * ourColor;
    FragColor = vec4(result, 0.75);
}

