#version  410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec4 vertCol;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec4 col;
out vec3 pos;

void main()
{
pos=vertPos.xyz;
gl_Position = P * V * M * vec4(vertPos,1);

}
