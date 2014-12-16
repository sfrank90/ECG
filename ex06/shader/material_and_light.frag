#version 330

struct LightSource {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 position;
};

struct Material {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  float specular_shininess;
};

// TODO: set up uniforms for multiple light sources //
uniform LightSource lightSources[10];
uniform int usedLightSources[10];

// uniform for the used material //
uniform Material material;

// fragment normal //
in vec3 vertexNormal;
// vector from fragment to camera //
in vec3 eyeDir;

// TODO: vector from fragment to light per light source //
in vec3 vLights[10];

// this defines the fragment output //
out vec4 color;

void main() {
  // normalize the vectors passed from your vertex program here //
  vec3 E = normalize(eyeDir);
  vec3 N = normalize(vertexNormal);
  
  // init the ambient, diffuse and specular color terms //
  vec3 ambientTerm = vec3(0);
  vec3 diffuseTerm = vec3(0);
  vec3 specularTerm = vec3(0);
  vec3 nLight = vec3(0);


  // TODO: compute the ambient, diffuse and specular color terms for every used light source //
  for(int i = 0; i < 10; ++i){
	if(usedLightSources[i] == 1){
		nLight = normalize(vLights[0]); 

		// TODO: compute the half-way-vector for our specular component //
		vec3 half = normalize(E+nLight);

		ambientTerm = ambientTerm + material.ambient_color * lightSources[i].ambient_color;
		diffuseTerm = diffuseTerm + material.diffuse_color * lightSources[i].diffuse_color;
		specularTerm = specularTerm + (material.specular_color * lightSources[i].specular_color) * pow(max(0.0, dot(N, half)), material.specular_shininess);
	}
  }

  // assign the final color to the fragment output variable //
  color = vec4(ambientTerm + diffuseTerm + specularTerm, 1);
}