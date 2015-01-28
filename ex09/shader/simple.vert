#version 330
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;

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

uniform LightSource lightSource;
uniform Material material;

// define out variables to be passed to the fragment shader //
out vec3 vertexNormal;
out vec3 eyeDir;
out vec3 lightDir;

out vec3 ambientColor;
out vec3 diffuseColor;
out vec3 specularColor;
out float specularShininess;

// modelview and projection matrix //
uniform mat4 view;
uniform mat4 modelview;
uniform mat4 projection;

void main() {
  // pass the light-material color information to the fragment program
  ambientColor = lightSource.ambient_color * material.ambient_color;
  diffuseColor = lightSource.diffuse_color * material.diffuse_color;
  specularColor = lightSource.specular_color * material.specular_color;
  specularShininess = material.specular_shininess;
  
  // create a normal matrix by inverting and transposing the modelview matrix //
  mat4 normalMatrix = transpose(inverse(modelview));
  // transform vertex position and the vertex normal using the appropriate matrices //
  
  vertexNormal = (normalMatrix * vec4(vertex_normal, 0)).xyz;
  vertexNormal = normalize(vertexNormal);

  // perform model view matrix
  gl_Position = modelview * vec4(vertex, 1.0);
  // perform projection matrix
  gl_Position = projection * gl_Position;

  // compute the vectors from the current vertex towards the camera and towards the light source //
  vec3 vertexInCamSpace = (modelview * vec4(vertex, 1.0)).xyz;
  vec3 lightInCamSpace = (view * vec4(lightSource.position, 1.0)).xyz;
  // vector from vertex to camera and from vertex to light //
  eyeDir = -vertexInCamSpace;
  lightDir = lightInCamSpace - vertexInCamSpace;
}