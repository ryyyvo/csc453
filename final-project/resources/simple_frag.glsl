#version 410 core 

out vec4 color;

in vec3 pos;


uniform vec4 extcol;



void main()
{
	
	color = extcol;
	
}
