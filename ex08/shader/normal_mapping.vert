#version 330

//sadly we need this to iterate over the lightsources on nvidia cards
//this might cause problems if other loops should not be unrolled
#pragma optionNV (unroll all)

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_texcoord;
// TODO: add vertex attribute locations for tangent and binormal
layout(location = 3) in vec3 vertex_tangent;
layout(location = 4) in vec3 vertex_binormal;


const int maxLightCount = 10;

// this struct helps to organize all the uniform parameters //
struct LightSource {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 position;
};

uniform LightSource lightSource[maxLightCount];
uniform int usedLightCount;

// out variables to be passed to the fragment shader //
out vec3 vertexNormal; // not needed anymore, when using normal maps //
out vec3 eyeDir;
out vec3 lightDir[maxLightCount];
out vec2 textureCoord;

// modelview and projection matrix //
uniform mat4 view;
uniform mat4 modelview;
uniform mat4 projection;

void main() {
  int lightCount = max(min(usedLightCount, maxLightCount), 0);
  
  // normal matrix //
  mat4 normalMatrix = transpose(inverse(modelview));
  
  gl_Position = projection * modelview * vec4(vertex, 1.0);
  
  // TODO: compute tangent space conversion matrix //

  vec4 normal = modelview * vec4(vertex_normal, 0.0);
  vec4 binormal = modelview * vec4(vertex_binormal, 0.0);
  vec4 tangent = modelview * vec4(vertex_tangent, 0.0);
  
  mat4 mInverse = mat4(tangent.x, binormal.x, normal.x, 0, tangent.y, binormal.y, normal.y, 0, tangent.z, binormal.z, normal.z, 0, 0, 0, 0, 0);

  // compute per vertex camera direction //
  vec3 vertexInCamSpace = (modelview * vec4(vertex, 1.0)).xyz;
  
  // TODO: vector from vertex to camera in *tangent space* //
  eyeDir = (mInverse* vec4(-vertexInCamSpace, 1.0)).xyz;

  // vertex to light for every light source! //
  for (int i = 0; i < maxLightCount; ++i) {
    vec3 lightInCamSpace = (view * vec4(lightSource[i].position, 1.0)).xyz;
    // TODO: vector from vertex to light in *tangent space* //
	lightDir[i] = (mInverse* vec4(lightInCamSpace - vertexInCamSpace, 1.0)).xyz;
  }
  
  // write texcoord //
  textureCoord = vertex_texcoord;
}