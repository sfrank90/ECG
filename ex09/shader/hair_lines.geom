// geometry shader for growing hair (lines)

#version 330

// TODO : create #define for number of output vertices 

// TODO : create layout
//	- triangles as input
//	- line strips as output, number set to previously defined number

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

// TODO: create uniforms for hair paramters (hairLength, gravity)

// TODO: create input and output variables
// - input : normal for each vertex of a triangle
// - output : one single color

void main(void)
{
	// TODO: compute center of triangle
	
	// TODO: compute normalized face normal
	
	// TODO: compute and emit hair vertices for each hair segment, which is processed in fragment shader
	// project triangle center and set for for output
	// - compute output color from material color
	//		-	for this task you can simply fake the shading by setting
	//			a simple gradient from first to last vertex of a hair
	// - compute vertex position for each hair segment
	//		-	begin at triangle center
	//		-	reduce y-component of normal in each step to simulate simple gravity effect
	//		-	normalize normal
	//		-	step into normal direction for each new segment vertex
	//		- emit vertex using EmitVertex()
	// - close primitive by calling EndPrimitive()

}
