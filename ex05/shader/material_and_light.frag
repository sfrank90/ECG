#version 330

// TODO: again define your variables passed from vertex to fragment program here
// - use 'in' as qualifier now
// - make sure to use the exakt same names
in vec3 vNormal;
in vec3 vCamera;
in vec3 vLight;

in vec3 vAmbient;
in vec3 vDiffuse;
in vec3 vSpecular;
in float fShininess;

// this defines the fragment output //
out vec4 color;

void main() {
  // TODO: normalize the vectors passed from your vertex program here //
  // - this needs to be done, because the interpolation of these vectors is linear //
  vec3 nNormal 	= normalize(vNormal);
  vec3 nCamera 	= normalize(vCamera);
  vec3 nLight  	= normalize(vLight);
  
  // TODO: compute the half-way-vector for our specular component //
  vec3 half = normalize(nCamera+nLight);

  // TODO: compute the ambient, diffuse and specular color terms as presented in the lecture //
  color = vec4(vDiffuse * max(0.0, dot(nNormal, nLight)), 1.0f);
  color+= vec4(vAmbient, 0.0f);
  color+= vec4(vSpecular * pow(max(0.0, dot(nNormal, half)), fShininess),0.0);
  // TODO: assign the final color to the fragment output variable //
  
}