// simple fragment shader that outputs hair color

#version 330

out vec4 fragColor;

in vec4 color;

void main()
{		
	fragColor = color;
}
