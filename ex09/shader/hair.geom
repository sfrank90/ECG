// geometry shader for growing hair (triangles)

#version 330

// TODO : create #define for number of output vertices 

// TODO : create layout
//	- triangles as input
//	- triangles as output, number set to previously defined number

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

// TODO: create input and output variables
// - input : normal for each vertex of a triangle
// - output : one single color

void main(void)
{
	// TODO: compute center vertex and normal of triangle
	
	// TODO: compute normal matrix
	
	// TODO: compute right vector in object space
	
	// TODO: compute right vector in view space
	
	// TODO: compute hair width and length based on respective input parameters and number of segments
	
	// TODO: compute center position in view space
		
	// TODO : compute output vertices
	//	- compute and emit first vertices of the hair
	//	- create for loop for remaining hair segments
	//	- compute new hair width
	//	- increment hair length
	//	- compute and draw shifted vertices along normal and right vector in view space
	//  - emit two segment vertices for each next segment
	//  - increment gravity
	//  - end primitive at the end
		

}
