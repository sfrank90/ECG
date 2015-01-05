#version 330

//sadly we need this to iterate over the lightsources on nvidia cards
//this might cause problems if other loops should not be unrolled
#pragma optionNV (unroll all)

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;
// TODO: add texture coordinates to your available vertex attributes //


const int maxLightCount = 10;

// these struct help to organize all the uniform parameters //
struct LightSource {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 position;
};

uniform LightSource lightSource[maxLightCount];
uniform int usedLightCount;

// out variables to be passed to the fragment shader //
out vec3 vertexNormal;
out vec3 eyeDir;
out vec3 lightDir[maxLightCount];
// TODO: add the texture coordinate as in/out variable to be passed to the fragment program //


// modelview and projection matrix //
uniform mat4 modelview;
uniform mat4 projection;

void main() {
  int lightCount = max(min(usedLightCount, maxLightCount), 0);
  
  // normal matrix //
  mat4 normalMatrix = transpose(inverse(modelview));
  
  // transform vertex position and the vertex normal using the appropriate matrices //
  vertexNormal = (normalMatrix * vec4(vertex_normal, 0)).xyz;
  gl_Position = projection * modelview * vec4(vertex, 1.0);
  
  // compute per vertex camera direction //
  vec3 vertexInCamSpace = (modelview * vec4(vertex, 1.0)).xyz;
  
  // vector from vertex to camera and from vertex to light //
  eyeDir = -vertexInCamSpace;
  
  // vertex to light for every light source! //
  for (int i = 0; i < maxLightCount; ++i) {
    vec3 lightInCamSpace = (modelview * vec4(lightSource[i].position, 1.0)).xyz;
    lightDir[i] = lightInCamSpace - vertexInCamSpace;
  }
  
  // TODO: write texcoord //
}