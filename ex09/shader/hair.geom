// geometry shader for growing hair (triangles)

#version 330

// TODO : create #define for number of output vertices 
#define N 10

// TODO : create layout
//	- triangles as input
//	- triangles as output, number set to previously defined number
layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices = N) out;

// material
struct Material {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  float specular_shininess;
};

uniform Material material;

// view, modelview and projection matrix //
uniform mat4 view;
uniform mat4 modelview;
uniform mat4 projection;

// TODO: create uniforms for hair paramters (hairLength, hairWidth, hairSegments, gravity)
uniform float hairLength;
uniform float hairWidth;
uniform int hairSegments;
uniform float gravity;

// TODO: create input and output variables
// - input : normal for each vertex of a triangle
// - output : one single color
in vec3 normal[];
out vec4 color;

void main(void)
{
	// TODO: compute center vertex and normal of triangle
	vec3 a = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);
	// center of triangle
    vec3 c = vec3(gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
	
	// TODO: compute normal matrix
	// normal of triangle
    vec3 n = normalize(normal[0] + normal[1] + normal[2]);

	mat4 normalMatrix = transpose(inverse(modelview));
	n = (normalMatrix * vec4(n, 0)).xyz;
    n = normalize(n);

	// TODO: compute right vector in object space
	vec3 right =  vec3(gl_in[0].gl_Position) - c;
	right = normalize(right);

	// TODO: compute right vector in view space
	vec4 right_vert =  view*vec4(right, 1.0);

	// TODO: compute hair width and length based on respective input parameters and number of segments
	float deltaWidth = hairWidth / float(hairSegments) / 2.0;
	float deltaLength = hairLength /  float(hairSegments);
	
	// TODO: compute center position in view space
	vec4 center_vert =  view * vec4(c, 1.0);
		
	// TODO : compute output vertices
	//	- compute and emit first vertices of the hair
	//	- create for loop for remaining hair segments
	//	- compute new hair width
	//	- increment hair length
	//	- compute and draw shifted vertices along normal and right vector in view space
	//  - emit two segment vertices for each next segment
	//  - increment gravity
	//  - end primitive at the end
	color = vec4(0.0);
    gl_Position = projection * modelview * vec4(c + (hairWidth/2.0)*right,1);
    EmitVertex();

	color = vec4(0.0);
    gl_Position = projection * modelview * vec4(c - (hairWidth/2.0)*right,1);
    EmitVertex();

	float t = deltaLength;
	float w = hairWidth / 2.0 - 2*deltaWidth;
    vec3 materialColor = material.diffuse_color;


	for(int i = 1; i < hairSegments; i++) {
	    color = vec4((i/float(hairSegments))*materialColor, 1);
		vec3 position = n * t + c.xyz + w*right.xyz;
			 position.y += -gravity * t * t;

	    gl_Position = projection * modelview * vec4(position, 1.0);
		EmitVertex();

		position = n * t + c.xyz - w*right.xyz;
	    position.y += -gravity * t * t;

	    gl_Position = projection * modelview * vec4(position, 1.0);
		EmitVertex();

		t += deltaLength;
		w -= deltaWidth;
	}
	EndPrimitive();	

}
