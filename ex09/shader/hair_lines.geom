// geometry shader for growing hair (lines)

#version 330

// TODO : create #define for number of output vertices 
#define N 10

// TODO : create layout
//	- triangles as input
//	- line strips as output, number set to previously defined number
layout(triangles) in;
layout(line_strip) out;
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

// TODO: create uniforms for hair paramters (hairLength, gravity)
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
	// TODO: compute center of triangle
	vec3 a = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);
	// center of triangle
    vec3 c = vec3(gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
	
	// TODO: compute normalized face normal
	// normal of triangle
    vec3 n = normalize(normal[0] + normal[1] + normal[2]);

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
	color = vec4(0.0);
    gl_Position = projection * modelview * vec4(c, 1.0);
    EmitVertex();

	float delta = hairLength / float(hairSegments);
	float t = delta;
	vec3 materialColor = material.diffuse_color;// + material.specular_color;
	for (int i = 1; i < hairSegments; ++i) {
		color = vec4((i/float(hairSegments))*materialColor, 1);
		vec3 position = n * t + c;
				position.y += -gravity * t * t;
		gl_Position = projection * modelview * vec4(position, 1.0);
		EmitVertex();
		t += delta;
	}
    EndPrimitive();

}
