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

#include "ObjLoader.h"
#include "CameraController.h"

#include <sstream>
#include <opencv/cv.h>
#include <opencv/highgui.h>

std::stack<glm::mat4> glm_ProjectionMatrix; 
std::stack<glm::mat4> glm_ModelViewMatrix; 

#define TASK 0
#define LOWRES 1

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
char* loadShaderSource(const char* fileName);
GLuint loadShaderFile(const char* fileName, GLenum shaderType);
// the used shader program //
GLuint shaderProgram = 0;
// this map stores uniform locations of our shader program //
std::map<std::string, GLint> uniformLocations;

// this struct helps to keep light source parameter uniforms together //
struct UniformLocation_Light {
  GLint ambient_color;
  GLint diffuse_color;
  GLint specular_color;
  GLint position;
};
// this map stores the light source uniform locations as 'UniformLocation_Light' structs //
std::map<std::string, UniformLocation_Light> uniformLocations_Lights;

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

// the program uses a list of materials and light sources, which can be chosen during rendering //
unsigned int materialIndex;
unsigned int materialCount;
std::vector<Material> materials;
unsigned int lightCount;
std::vector<LightSource> lights;

// #INFO# predefined enum for available texture types //
enum TextureLayer {DIFFUSE = 0, EMISSIVE, SKY_ALPHA, SKY_COLOR, NORMAL, LAYER_COUNT};
// #INFO# Container for texture data //
struct Texture {
  Texture() : isEnabled(false), data(NULL), width(0), height(0), glTextureLocation(0), uniformLocation(-1), uniformEnabledLocation(-1) {};
  // is this texture used ?
  bool isEnabled;
  // local data storage
  unsigned char *data;
  // texture size
  unsigned int width;
  unsigned int height;
  // OpenGL texture handle
  GLuint glTextureLocation;
  // GLSL texture handle (uniform access in the shader)
  GLint uniformLocation;
  // GLSL handles to boolean variable (allows to toggle texture in shader) (optional)
  GLint uniformEnabledLocation;
};
// #INFO# array to store textures //
Texture texture[LAYER_COUNT];
// method to load a texture from a given file
void loadTextureData(const char *fileName, Texture &texture);
// method to initialize the texture object
void initTextures();


// window controls //
void updateGL();
void idle();
void keyDownEvent(unsigned char key, int x, int y);
void keyUpEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// camera controls //
CameraController camera(0, M_PI/4, 10);

// viewport //
GLint windowWidth, windowHeight;

// geometry //
void initScene();
void renderScene();

// OBJ import //
ObjLoader objLoader;

// #INFO# this allows to switch between exercises (0 -> Task 8.1, 1 -> Task 8.2) //
int task;

int CheckGLErrors() {
  int errCount = 0;
  for(GLenum currError = glGetError(); currError != GL_NO_ERROR; currError = glGetError()) {
    std::stringstream sstr;
    
    switch (currError) {
      case GL_INVALID_ENUM : sstr << "GL_INVALID_ENUM"; break;
      case GL_INVALID_VALUE : sstr << "GL_INVALID_VALUE"; break;
      case GL_INVALID_OPERATION : sstr << "GL_INVALID_OPERATION"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION : sstr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
      case GL_OUT_OF_MEMORY : sstr << "GL_OUT_OF_MEMORY"; break;
      default : sstr << "unknown error (" << currError << ")";
    }
    std::cout << "found error: " << sstr.str() << std::endl;
    ++errCount;
  }
 
  return errCount;
}


int main (int argc, char **argv) {
  task = TASK;
  if (argc > 1) {
    // #INFO# //
    // 0 -> render multi texture exercise 8.1
    // 1 -> render normal map exercise 8.2 using moon textures
    // 2 -> render normal map exercise 8.2 using mars textures
    task = atoi(argv[1]);
  }
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  windowWidth = 512;
  windowHeight = 512;
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Exercise 08 - Multi-Texturing & Normal Maps");
  
  glutDisplayFunc(updateGL);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyDownEvent);
  glutKeyboardUpFunc(keyUpEvent); 
  glutMouseFunc(mouseEvent);
  glutMotionFunc(mouseMoveEvent);
  
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  
  // init stuff //
  initGL();
  
  // init matrix stacks with identity //
  glm_ProjectionMatrix.push(glm::mat4(1));
  glm_ModelViewMatrix.push(glm::mat4(1));
  
  initShader();
  initScene();
  initTextures();
  
  // start render loop //
  if (enableShader()) {
    glutMainLoop();
    disableShader();
    
    // clean up allocated data //
    deleteShader();
  }
  std::cin.get();

  return 0;
}

void initGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

std::string getUniformStructLocStr(const std::string &structName, const std::string &memberName, int arrayIndex = -1) {
  std::stringstream sstr("");
  sstr << structName;
  if (arrayIndex >= 0) {
    sstr << "[" << arrayIndex << "]";
  }
  sstr << "." << memberName;
  return sstr.str();
}

void initShader() {
  shaderProgram = glCreateProgram();
  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
  
  
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;
  
  if (task == 0) {
    vertexShader = loadShaderFile("../../shader/multi_texture.vert", GL_VERTEX_SHADER);
    fragmentShader = loadShaderFile("../../shader/multi_texture.frag", GL_FRAGMENT_SHADER);
  } else {
    vertexShader = loadShaderFile("../../shader/normal_mapping.vert", GL_VERTEX_SHADER);
    fragmentShader = loadShaderFile("../../shader/normal_mapping.frag", GL_FRAGMENT_SHADER);
  }
  
  if (vertexShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  if (fragmentShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  
  // successfully loaded and compiled shaders -> attach them to program //
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  
  // mark shaders for deletion after clean up (they will be deleted, when detached from all shader programs) //
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  
  // link shader program //
  glLinkProgram(shaderProgram);
  
  // get log //
  int logMaxLength;
  glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logMaxLength);
  char *log = new char[logMaxLength];
  int logLength = 0;
  glGetProgramInfoLog(shaderProgram, logMaxLength, &logLength, log);
  if (logLength > 0) {
    std::cout << "(initShader) - Linker log:\n------------------\n" << log << "\n------------------" << std::endl;
  }
  delete[] log;
  
  // set address of fragment color output //
  glBindFragDataLocation(shaderProgram, 0, "color");
  
  // get uniform locations for common variables //
  uniformLocations["projection"] = glGetUniformLocation(shaderProgram, "projection");
  uniformLocations["modelview"] = glGetUniformLocation(shaderProgram, "modelview");
  uniformLocations["view"] = glGetUniformLocation(shaderProgram, "view");
  
  // material unform locations //
  uniformLocations["material.ambient"] = glGetUniformLocation(shaderProgram, "material.ambient_color");
  uniformLocations["material.diffuse"] = glGetUniformLocation(shaderProgram, "material.diffuse_color");
  uniformLocations["material.specular"] = glGetUniformLocation(shaderProgram, "material.specular_color");
  uniformLocations["material.shininess"] = glGetUniformLocation(shaderProgram, "material.specular_shininess");
  
  // store the uniform locations for all light source properties
  for (int i = 0; i < 10; ++i) {
    UniformLocation_Light lightLocation;
    lightLocation.ambient_color = glGetUniformLocation(shaderProgram, getUniformStructLocStr("lightSource", "ambient_color", i).c_str());
    lightLocation.diffuse_color = glGetUniformLocation(shaderProgram, getUniformStructLocStr("lightSource", "diffuse_color", i).c_str());
    lightLocation.specular_color = glGetUniformLocation(shaderProgram, getUniformStructLocStr("lightSource", "specular_color", i).c_str());
    lightLocation.position = glGetUniformLocation(shaderProgram, getUniformStructLocStr("lightSource", "position", i).c_str());
    
    std::stringstream sstr("");
    sstr << "light_" << i;
    uniformLocations_Lights[sstr.str()] = lightLocation;
  }
  uniformLocations["usedLightCount"] = glGetUniformLocation(shaderProgram, "usedLightCount");
  
  // TODO: get texture uniform locations and store them in the texture containers //
  if (task == 0) {
    // TODO: Task 8.1
    // TODO: get the texture uniform locations of the textures defined in 'multi_texture.frag' //
	  texture[0].uniformLocation = glGetUniformLocation(shaderProgram, "diffuseTex");
	  texture[1].uniformLocation = glGetUniformLocation(shaderProgram, "emissiveTex");
	  texture[2].uniformLocation = glGetUniformLocation(shaderProgram, "skyTex");
	  texture[3].uniformLocation = glGetUniformLocation(shaderProgram, "alphaTex");
  } else {
    // TODO: Task 8.2
    // TODO: get the texture uniform locations of the textures defined in 'normal_mapping.frag' //
	  texture[0].uniformLocation = glGetUniformLocation(shaderProgram, "colorTex");
	  texture[1].uniformLocation = glGetUniformLocation(shaderProgram, "normalTex");
  }
}

bool enableShader() {
  if (shaderProgram > 0) {
    glUseProgram(shaderProgram);
  } else {
    std::cout << "(enableShader) - Shader program not initialized." << std::endl;
  }
  return shaderProgram > 0;
}

void disableShader() {
  glUseProgram(0);
}

void deleteShader() {
  // use standard pipeline //
  glUseProgram(0);
  // delete shader program //
  glDeleteProgram(shaderProgram);
  shaderProgram = 0;
}

// load and compile shader code //
char* loadShaderSource(const char* fileName) {
  char *shaderSource = NULL;
  
  std::ifstream file(fileName, std::ios::in);
  if (file.is_open()) {
    unsigned long srcLength = 0;
    file.tellg();
    file.seekg(0, std::ios::end);
    srcLength = file.tellg();
    file.seekg(0, std::ios::beg);
    shaderSource = new char[srcLength+1];
    file.read(shaderSource, srcLength);
    shaderSource[srcLength] = '\0';
    file.close();
  } else {
    std::cout << "(loadShaderSource) - Could not open file \"" << fileName << "\"." << std::endl;
  }
  
  return shaderSource;
}

// Print information about the compiling step
void printShaderInfoLog(GLuint shader)
{
	GLint infologLength = 0;
	GLsizei charsWritten = 0;
	char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
	infoLog = (char *)malloc(infologLength);
	glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
	printf("%s\n", infoLog);
	free(infoLog);
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
	if (shaderStream.is_open()){
		std::string line = "";
		while (std::getline(shaderStream, line))
			shaderCode += "\n" + line;
		shaderStream.close();
	}
	else {
		printf("Impossible to open %s. Please check your directories !\n", fileName);
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

// initialize an OpenGL texture objects //
void initTextures (void) {
  if (task == 0) {
    // TODO: Task 8.1
    // TODO: Load earth textures and assign them to the proper texture containers
	//		low-resolution
    //       - earthmap_1k.jpg
    //       - earthlights_1k.jpg
    //       - earthcloudmaptrans_1k.jpg
    //       - earthcloudmap_1k.jpg
	//
	//		high-resolution
    //       - earthmap_8k.jpg
    //       - earthlights_8k.jpg
    //       - earthcloudmaptrans_8k.jpg
    //       - earthcloudmap_8k.jpg
	//


	  if (LOWRES) {
		  loadTextureData("../../textures/earthmap_1k.jpg", texture[0]);
		  loadTextureData("../../textures/earthlights_1k.jpg", texture[1]);
		  loadTextureData("../../textures/earthcloudmap_1k.jpg", texture[2]);
		  loadTextureData("../../textures/earthcloudmaptrans_1k.jpg", texture[3]);
	  }
	  else{
		  loadTextureData("../../highresTextures/earthmap_8k.png", texture[0]);
		  loadTextureData("../../highresTextures/earthlights_8k.png", texture[1]);
		  loadTextureData("../../highresTextures/earthcloudmap_8k.png", texture[2]);
		  loadTextureData("../../highresTextures/earthcloudmaptrans_8k.png", texture[3]);
	  }

  } else if (task == 1) {
    // TODO: Task 8.2
    // TODO: Load moon textures and assign them to the proper texture containers
    //       - moon.png
    //       - moon_normal.png
	  loadTextureData("../../textures/moon.png", texture[0]);
	  loadTextureData("../../textures/moon_normal.png", texture[1]);

  } else {
    // TODO: Task 8.2
    // TODO: Load mars textures and assign them to the proper texture containers
    //       - mars.png
    //       - mars_normal.png
	  loadTextureData("../../textures/mars.png", texture[0]);
	  loadTextureData("../../textures/mars_normal.png", texture[1]);
  }
  
  // #INFO#
  // this creates an OpenGL texture, if data has been loaded into a container object of the given texture layer index (see enum above) //
  for (unsigned int layer = DIFFUSE; layer < LAYER_COUNT; ++layer) {
    if (texture[layer].data != NULL) {
      // texture has been successfully loaded //
      glGenTextures(1, &texture[layer].glTextureLocation);
      
      glBindTexture(GL_TEXTURE_2D, texture[layer].glTextureLocation);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture[layer].width, texture[layer].height, 0, GL_BGR, GL_UNSIGNED_BYTE, texture[layer].data);
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
      
      // clean up local texture data //
      delete[] texture[layer].data;
    }
  }
}

// loads texture data from disk //
void loadTextureData(const char *textureFile, Texture &texture) {
  IplImage *image = cvLoadImage(textureFile, CV_LOAD_IMAGE_COLOR);
  if (image != NULL) {
    // flip image vertically //
    cvFlip(image);
    texture.width = image->width;
    texture.height = image->height;
    if (texture.data) {
      delete[] texture.data;
      texture.data = NULL;
    }
    texture.data = new unsigned char[image->imageSize];
    memcpy(texture.data, image->imageData, image->imageSize);
    texture.isEnabled = true;
	std::cout << "(loadTextureData) : reading from \"" << textureFile << "\" succeeded." << std::endl;
  } else {
    texture.isEnabled = false;
    std::cout << "(loadTextureData) : reading from \"" << textureFile << "\" failed." << std::endl;
  }
  cvReleaseImage(&image);
}

void initScene() {
  // load scene.obj from disk and create renderable MeshObj //
  objLoader.loadObjFile("../../meshes/sphere.obj", "sceneObject");
  
  // init materials //
  Material mat;
  mat.ambient_color = glm::vec3(1.0, 1.0, 1.0);
  mat.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_color = glm::vec3(0.5, 0.5, 0.5);
  mat.specular_shininess = 5.0;
  materials.push_back(mat);
  
  // save material count for later and select first material //
  materialCount = materials.size();
  materialIndex = 0;
  
  // init lights //
  LightSource light;
  light.ambient_color = glm::vec3(0.05, 0.05, 0.05);
  light.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
  light.specular_color = glm::vec3(1.0, 1.0, 1.0);
  
  light.position = glm::vec3(15, 15, 15);
  lights.push_back(light);
  
  // save light source count for later and select first light source //
  lightCount = lights.size();
}

void setupLightAndMaterial() {
  // uploads the properties of the currently active light sources here //
  int shaderLightIdx = 0;
  for (unsigned int i = 0; i < lightCount; ++i) {
    if (lights[i].enabled) {
      std::stringstream sstr("");
      sstr << "light_" << shaderLightIdx;
      UniformLocation_Light &light = uniformLocations_Lights[sstr.str()];
      glUniform3fv(light.position, 1, glm::value_ptr(lights[i].position));
      glUniform3fv(light.ambient_color, 1, glm::value_ptr(lights[i].ambient_color));
      glUniform3fv(light.diffuse_color, 1, glm::value_ptr(lights[i].diffuse_color));
      glUniform3fv(light.specular_color, 1, glm::value_ptr(lights[i].specular_color));
      ++shaderLightIdx;
    }
  }
  glUniform1i(uniformLocations["usedLightCount"], shaderLightIdx);
  
  // uploads the chosen material properties here //
  glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
  glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
  glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
  glUniform1f(uniformLocations["material.shininess"], materials[materialIndex].specular_shininess);
}

// TODO: complete the code to render a multitextured earth //
// - upload diffuse, emissive, alpha and sky-color textures
void renderEarth() {
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  
  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(10));
  
  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  
  // TODO: upload textures to individual texture units //
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture[0].glTextureLocation);
  glUniform1i(texture[0].uniformLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[1].glTextureLocation);
  glUniform1i(texture[1].uniformLocation, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture[2].glTextureLocation);
  glUniform1i(texture[2].uniformLocation, 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, texture[3].glTextureLocation);
  glUniform1i(texture[3].uniformLocation, 3);
  
  // render the actual object //
  MeshObj *mesh = objLoader.getMeshObj("sceneObject");
  mesh->render();
  
  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
}

// TODO: complete the code to render the moon using a normal map //
// - upload diffuse and normal map texture
void renderMoon() {
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  
  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(10));
  
  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  
  // TODO: upload textures to individual texture units //
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture[0].glTextureLocation);
  glUniform1i(texture[0].uniformLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture[1].glTextureLocation);
  glUniform1i(texture[1].uniformLocation, 1);
  // render the actual object //
  MeshObj *mesh = objLoader.getMeshObj("sceneObject");
  mesh->render();
  
  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
}

void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // update camera
  camera.updateCamera();

  // set viewport dimensions //
  glViewport(0, 0, windowWidth, windowHeight);
  
  // get projection mat from camera controller //
  glm_ProjectionMatrix.top() = camera.getProjectionMat();
  // upload projection matrix //
  glUniformMatrix4fv(uniformLocations["projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
  glUniformMatrix4fv(uniformLocations["view"], 1, false, glm::value_ptr(camera.getModelViewMat()));
  
  // init scene graph by cloning the top entry, which can now be manipulated //
  // get modelview mat from camera controller //
  glm_ModelViewMatrix.top() = camera.getModelViewMat();
  
  // setup light and material in shader //
  setupLightAndMaterial();

  // #INFO# render scene //
  if (task == 0) {
    renderEarth();
  } else {
    renderMoon();
  }
  
  // swap renderbuffers for smooth rendering //
  glutSwapBuffers();
}

void idle() {
  glutPostRedisplay();
}

// toggles a light source on or off //
void toggleLightSource(unsigned int i) {
  if (i < lightCount) {
    lights[i].enabled = !lights[i].enabled;
  }
}

void keyUpEvent(unsigned char key, int x, int y) {  
	
  switch (key) {
    case 'w': {
      // move forward //
      camera.stop(CameraController::MOVE_FORWARD);
      break;
    }
    case 's': {
      // move backward //
      camera.stop(CameraController::MOVE_BACKWARD);
      break;
    }
    case 'a': {
      // move left //
      camera.stop(CameraController::MOVE_LEFT);
      break;
    }
    case 'd': {
      // move right //
      camera.stop(CameraController::MOVE_RIGHT);
      break;
    }
    case 'r': {
      // move up //
      camera.stop(CameraController::MOVE_UP);
      break;
    }
    case 'f': {
      // move down //
      camera.stop(CameraController::MOVE_DOWN);
      break;
    }
  }
  glutPostRedisplay();
}  

void keyDownEvent(unsigned char key, int x, int y) {
  switch (key) {
    case 'x':
    case 27 : {
      exit(0);
      break;
    }
    case 'w': {
      // move forward //
      camera.move(CameraController::MOVE_FORWARD);
      break;
    }
    case 's': {
      // move backward //
      camera.move(CameraController::MOVE_BACKWARD);
      break;
    }
    case 'a': {
      // move left //
      camera.move(CameraController::MOVE_LEFT);
      break;
    }
    case 'd': {
      // move right //
      camera.move(CameraController::MOVE_RIGHT);
      break;
    }
    case 'r': {
      // move up //
      camera.move(CameraController::MOVE_UP);
      break;
    }
    case 'f': {
      // move down //
      camera.move(CameraController::MOVE_DOWN);
      break;
    }
    case 'm': {
      materialIndex++;
      if (materialIndex >= materialCount) materialIndex = 0;
      break;
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      int lightIdx;
      std::stringstream keyStr;
      keyStr << key;
      keyStr >> lightIdx;
      if (lightIdx == 0) lightIdx = 10;
      if (lightIdx > 0) toggleLightSource(lightIdx - 1);
      break;
    }
  }
  glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y) {
  CameraController::MouseState mouseState;
  if (state == GLUT_DOWN) {
    switch (button) {
      case GLUT_LEFT_BUTTON : {
        mouseState = CameraController::LEFT_BTN;
        break;
      }
      case GLUT_RIGHT_BUTTON : {
        mouseState = CameraController::RIGHT_BTN;
        break;
      }
	  default: {
		  mouseState = CameraController::NO_BTN;
		  break;
	  }
    }
  } else {
    mouseState = CameraController::NO_BTN;
  }
  camera.updateMouseBtn(mouseState, x, y);
  glutPostRedisplay();
}

void mouseMoveEvent(int x, int y) {
  camera.updateMousePos(x, y);
  glutPostRedisplay();
}

