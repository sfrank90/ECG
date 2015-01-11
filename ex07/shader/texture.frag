#version 330
const int maxLightCount = 10;

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

uniform LightSource lightSource[maxLightCount];
uniform int usedLightCount;
uniform Material material;

uniform int useBG;
uniform float height;

// variables passed from vertex to fragment program //
in vec3 vertexNormal;
in vec3 eyeDir;
in vec3 lightDir[maxLightCount];
// TODO: add a texture coordinate as a vertex attribute //
in vec2 tc;

// texture //
// TODO: set up a texture uniform //
uniform sampler2D tex;

// this defines the fragment output //
out vec4 color;

void main() {
  // TODO: get the texel value from your texture at the position of the passed texture coordinate //
  vec4 texColor = texture2D(tex,vec2(tc.x, 1-tc.y));

  int lightCount = max(min(usedLightCount, maxLightCount), 0);
  // normalize the vectors passed from your vertex program //
  vec3 E = normalize(eyeDir);
  vec3 N = normalize(vertexNormal);
  
  // compute the ambient, diffuse and specular color terms //
  vec3 ambientTerm = vec3(0);
  vec3 diffuseTerm = vec3(0);
  vec3 specularTerm = vec3(0);
  vec3 L, H;
  for (int i = 0; i < lightCount; ++i) {
    L = normalize(lightDir[i]);
    H = normalize(E + L);
    ambientTerm += lightSource[i].ambient_color;
    diffuseTerm += lightSource[i].diffuse_color * max(dot(L, N), 0);
    specularTerm += lightSource[i].specular_color * pow(max(dot(H, N), 0), material.specular_shininess);
  }
  ambientTerm *= material.ambient_color;
  diffuseTerm *= material.diffuse_color;
  specularTerm *= material.specular_color;
  
  // assign the final color to the fragment output variable //
  // TODO: combine the light/material color and the texture color properly //
	if(useBG != 0) {
		color = mix(vec4(0,0,0,1), vec4(1,1,1,1), gl_FragCoord.y/height);
	} else {
		color = vec4(ambientTerm + diffuseTerm*texColor.rgb + specularTerm, 1);
  }
}