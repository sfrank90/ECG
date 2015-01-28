#version 330

// variables passed from vertex to fragment program
in vec3 vertexNormal;
in vec3 eyeDir;
in vec3 lightDir;
in vec3 ambientColor;
in vec3 diffuseColor;
in vec3 specularColor;
in float specularShininess;

// this defines the fragment output //
out vec4 color;

void main() {
  // normalize the vectors from vertex program //
  vec3 E = normalize(eyeDir);
  vec3 L = normalize(lightDir);
  vec3 N = normalize(vertexNormal);
  
  // compute the half-way-vector for specular component //
  vec3 H = normalize(E + L);
  
  // compute the ambient, diffuse and specular color terms //
  vec3 ambientTerm = ambientColor;
  vec3 diffuseTerm = diffuseColor * max(dot(L, N), 0);
  vec3 specularTerm = specularColor * pow(max(dot(H, N), 0), specularShininess);
  
  // assign the final color to the fragment output variable //
  color = vec4(ambientTerm + diffuseTerm + specularTerm, 1);
}