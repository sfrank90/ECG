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

// variables passed from vertex to fragment program //
in vec3 vertexNormal;
in vec3 eyeDir;
in vec3 lightDir[maxLightCount];
in vec2 textureCoord;

// texture //
// TODO: load multiple textures for diffuse color, emissive light, sky color and alpha mask //
uniform sampler2D diffuseTex;
uniform sampler2D emissiveTex;
uniform sampler2D skyTex;
uniform sampler2D alphaTex;

// this defines the fragment output //
out vec4 color;

void main() {
  // TODO: load values from textures //
  vec4 diffuseColor = texture(diffuseTex,vec2(textureCoord.x, textureCoord.y));
  vec4 emissiveLight = texture(emissiveTex,vec2(textureCoord.x, textureCoord.y));
  vec4 skyColor = texture(skyTex,vec2(textureCoord.x, textureCoord.y));
  vec4 alphaMask = texture(alphaTex,vec2(textureCoord.x, textureCoord.y));

  // light computation //
  int lightCount = max(min(usedLightCount, maxLightCount), 0);
  // normalize the vectors passed from your vertex program //
  vec3 E = normalize(eyeDir);
  vec3 N = normalize(vertexNormal);
  
  // compute the ambient, diffuse and specular color terms //
  vec3 ambientTerm = vec3(0);
  vec3 diffuseTerm = vec3(0);
  vec3 specularTerm = vec3(0);
  float emissiveTerm = 0;

  vec3 L, H;
  for (int i = 0; i < lightCount; ++i) {
    L = normalize(lightDir[i]);
    H = normalize(E + L);
    ambientTerm += lightSource[i].ambient_color;
    diffuseTerm += lightSource[i].diffuse_color * max(dot(L, N), 0);
    specularTerm += lightSource[i].specular_color * pow(max(dot(H, N), 0), material.specular_shininess);
    // TODO: compute emissive texture component based on incident light angle //
	emissiveTerm += max(dot(L, N), 0);
  }
  emissiveTerm += 0.8;
  emissiveTerm = min(emissiveTerm,1);
  emissiveTerm = pow(emissiveTerm,5);
  ambientTerm *= material.ambient_color;
  diffuseTerm *= material.diffuse_color;
  specularTerm *= material.specular_color;

  diffuseColor = (diffuseColor*(vec4(1)-alphaMask )+ skyColor*alphaMask) ;
  diffuseColor *= vec4(diffuseTerm,0);
  diffuseColor += vec4(specularTerm,0);
  // TODO: assign the final color to the fragment output variable //
  color = diffuseColor + emissiveLight*(1-emissiveTerm);
}