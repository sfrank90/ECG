#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp> 

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <cmath>
#include <sstream>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

// function definition
bool glError(char *msg);
void initGL();
void printShaderInfoLog(GLuint shader);
void printProgramInfoLog(GLuint program);
GLuint loadShaderFile(const char* fileName, GLenum shaderType);
void createShader(GLuint &shaderProgram, std::string vertexshader_filename, std::string fragmentshader_filename);
void createShader(GLuint &shaderProgram, std::string vertexshader_filename, std::string geometryshader_filename, std::string fragmentshader_filename);
void disableShader();
void deleteShader();

// this struct helps to keep light source parameter uniforms together //
struct UniformLocation_Light {
  GLint ambient_color;
  GLint diffuse_color;
  GLint specular_color;
  GLint position;
};

// these structs are also used in the shader code  //
// this helps to access the parameters more easily //
struct Material {
  glm::vec3 ambient_color;
  glm::vec3 diffuse_color;
  glm::vec3 specular_color;
  float specular_shininess;
};

struct LightSource {
  LightSource() : enabled(true) {};
  bool enabled;
  glm::vec3 ambient_color;
  glm::vec3 diffuse_color;
  glm::vec3 specular_color;
  glm::vec3 position;
};


void initGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

bool glError(char *msg) {
  
  bool err_occured = false;
  for(GLenum currError = glGetError(); currError != GL_NO_ERROR; currError = glGetError()) {
    std::stringstream sstr;
	err_occured = true;
    switch (currError) {
      case GL_INVALID_ENUM : sstr << "GL_INVALID_ENUM"; break;
      case GL_INVALID_VALUE : sstr << "GL_INVALID_VALUE"; break;
      case GL_INVALID_OPERATION : sstr << "GL_INVALID_OPERATION"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION : sstr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
      case GL_OUT_OF_MEMORY : sstr << "GL_OUT_OF_MEMORY"; break;
      default : sstr << "unknown error (" << currError << ")";
    }
    std::cout << "found error: " << sstr.str() << std::endl;
    
  }
 
  return err_occured;
}

// Print information about the compiling step
void printShaderInfoLog(GLuint shader)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH,&infologLength);		
	infoLog = (char *)malloc(infologLength);
	glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
	free(infoLog);
}

// Print information about the linking step
void printProgramInfoLog(GLuint program)
{
	GLint infoLogLength = 0;
	GLsizei charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infoLogLength);
	infoLog = (char *)malloc(infoLogLength);
	glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
	free(infoLog);
}

void disableShader() {
  glUseProgram(0);
}

void deleteShader(GLuint &shader) {
  // use standard pipeline //
  glUseProgram(0);
  // delete shader program //
  glDeleteProgram(shader);
  shader = 0;
}

// loads a source file and directly compiles it to a shader of 'shaderType' //
GLuint loadShaderFile(const char* fileName, GLenum shaderType) {
  GLuint shader = glCreateShader(shaderType);
  // check if operation failed //
  if (shader == 0) {
    std::cout << "(loadShaderFile) - Could not create shader." << std::endl;
    return 0;
  }
  
  // load source code from file //
  std::string shaderCode;
  std::ifstream shaderStream(fileName, std::ios::in);
  if(shaderStream.is_open()){
	std::string line = "";
	while(std::getline(shaderStream, line))
		shaderCode += "\n" + line;
	shaderStream.close();
  }
  else {
	printf("(loadShaderFile) - Impossible to open %s. Please check your directories !\n", fileName);
	return 0;
  }
  char const * shaderSrc = shaderCode.c_str();

  if (shaderSrc == NULL) return 0;
  // pass source code to new shader object //
  glShaderSource(shader, 1, &shaderSrc, NULL);

  // compile shader //
  glCompileShader(shader);
    
  // log compile messages, if any //
  printShaderInfoLog(shader);
    
  // return compiled shader (may have compiled WITH errors) //
  return shader;
}

void createShader(GLuint &shaderProgram, std::string vertexshader_filename, std::string fragmentshader_filename) {
	createShader(shaderProgram, vertexshader_filename, "", fragmentshader_filename);
}

void createShader(GLuint &shaderProgram, std::string vertexshader_filename, std::string geometryshader_filename, std::string fragmentshader_filename) {

  shaderProgram = glCreateProgram();

  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
    
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;
  // TODO: create handle for geometry shader

  // TODO: check if geometry shader should be created actually

  vertexShader = loadShaderFile(vertexshader_filename.c_str(), GL_VERTEX_SHADER);
  fragmentShader = loadShaderFile(fragmentshader_filename.c_str(), GL_FRAGMENT_SHADER);
  
  // TODO: load geometry shader if necessary
    
  if (vertexShader == 0) {
    std::cout << "(createShader) - Could not create vertex shader." << std::endl;
    deleteShader(shaderProgram);
    return;
  }
  if (fragmentShader == 0) {
    std::cout << "(createShader) - Could not create fragment shader." << std::endl;
    deleteShader(shaderProgram);
    return;
  }
  // TODO: check if geometry shader has been created successfully, if necessary
  
  
  // successfully loaded and compiled shaders -> attach them to program //
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  // TODO: attach geometry shader, if necessary
    
  // mark shaders for deletion after clean up (they will be deleted, when detached from all shader programs) //
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  // TODO: clean up geometry shader if necessary
  
  // link shader program //
  glLinkProgram(shaderProgram);
  
  // get log //
  printProgramInfoLog(shaderProgram);
}

bool enableShader(GLuint &shader) {
  if (shader > 0) {
    glUseProgram(shader);
  } else {
    std::cout << "(enableShader) - Shader program not initialized." << std::endl;
  }
  return shader > 0;
}

#endif