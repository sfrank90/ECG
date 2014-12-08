#version 330
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;

// these struct help to organize all the uniform parameters //
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

// TODO: define out variables to be passed to the fragment shader //
// - define variables for vertex normal, camera vector and light vector
// - also define variables to pass color information to the fragment shader
//   (ambient, diffuse and specular color and the shininess exponent)
out vec3 vNormal;
out vec3 vCamera;
out vec3 vLight;

out vec3 vAmbient;
out vec3 vDiffuse;
out vec3 vSpecular;
out float fShininess;

// view, modelview and projection matrix //
uniform mat4 view;
uniform mat4 modelview;
uniform mat4 projection;

// TODO: define uniform variables used for the pumping effect //

void main() {
  // TODO: pass the light-material color information to the fragment program
  // - as presented in the lecture, you just need to combine light and material color here
  // - assign the final values to your defined out-variables
  vAmbient   = material.ambient_color * lightSource.ambient_color;
  vDiffuse   = material.diffuse_color * lightSource.diffuse_color;
  vSpecular  = material.specular_color * lightSource.specular_color;
  fShininess = material.specular_shininess;

  // TODO: create a normal matrix by inverting and transposing the modelview matrix //
  mat4 normalMat = transpose(inverse(modelview));

  // TODO: transform vertex position and the vertex normal using the appropriate matrices //
  // - assign the transformed vertex position (modelview & projection) to 'gl_Position'
  // - assign the transformed vertex normal (normal matrix) to your out-variable as defined above
  // - let the model pump by transforming the vertex
  vec3 v = vertex + pumpAmplitude*abs(sin(0.125*time))*vertex_normal;
  gl_Position = projection * modelview * vec4(v,1.0);
  vNormal = (normalMat * vec4(vertex_normal,0.0f)).xyz;

  // TODO: compute the vectors from the current vertex towards the camera and towards the light source //
  vLight  = ((view * vec4(lightSource.position,1.0f)) - (modelview * vec4(v,1.0f))).xyz;
  vCamera = -(modelview * vec4(v,1.0f)).xyz;
}