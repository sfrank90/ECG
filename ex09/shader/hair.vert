#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;

// TODO: define normal for output
out vec3 normal;
void main()
{	
	// TODO: pass normal and vertex to geometry shader
	normal = vertex_normal;
	gl_Position = vec4(vertex,1);
}
