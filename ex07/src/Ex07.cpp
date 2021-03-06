//#define EX7_4

#include "Ex07.h"
#include <sstream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
// #include <opencv/cv.h>
// #include <opencv/highgui.h>

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
bool glError(char *msg);
void printShaderInfoLog(GLuint shader);
void printProgramInfoLog(GLuint program);
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

unsigned int materialIndexMegatron = 0;
unsigned int materialIndexOptimus = 1;

// window controls //
int CheckGLErrors();
void updateGL();
void idle();
void close();
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// camera controls //
CameraController camera(0, M_PI/4, 5);

// viewport //
GLint windowWidth, windowHeight;

// geometry //
void initScene();
void renderScene();

// textures //
void initTextures();
TextureData loadTextureData(const char *fileName);
std::map<std::string, TextureData> textures;

// OBJ import //
ObjLoader *objLoader = 0;

int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  windowWidth = 512;
  windowHeight = 512;
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Exercise 07 - Textures");
  
  glutDisplayFunc(updateGL);
  glutReshapeFunc(Reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboardEvent);
  glutMouseFunc(mouseEvent);
  glutMotionFunc(mouseMoveEvent);
  glutCloseFunc(close);
  
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  
  // init stuff //
  initGL();

  // init object loader
  objLoader = new ObjLoader();
  
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
  
  return 0;
}

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

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
	windowWidth = width;
	windowHeight = height;

}

void close() {

	// free allocated objects
	delete objLoader;

	std::cout << "Shutdown program." << std::endl;
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

void initShader() {
  shaderProgram = glCreateProgram();
  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
  
  GLuint vertexShader = loadShaderFile("../../shader/texture.vert", GL_VERTEX_SHADER);
  if (vertexShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  GLuint fragmentShader = loadShaderFile("../../shader/texture.frag", GL_FRAGMENT_SHADER);
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
  printProgramInfoLog(shaderProgram);
  
  // set address of fragment color output //
  glBindFragDataLocation(shaderProgram, 0, "color");
  
  // get uniform locations for common variables //
  uniformLocations["projection"] = glGetUniformLocation(shaderProgram, "projection");
  uniformLocations["modelview"] = glGetUniformLocation(shaderProgram, "modelview");
  
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
  
  // TODO: get texture uniform location //
  uniformLocations["awesomeTexture"] = glGetUniformLocation(shaderProgram, "tex");

  uniformLocations["usebg"] = glGetUniformLocation(shaderProgram, "useBG");
  uniformLocations["height"] = glGetUniformLocation(shaderProgram, "height");
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

// TODO: initialize an OpenGL texture object //
// - load the image data from a file using "loadTextureData"
// - generate a new OpenGL texture
// - initialize the texture properly (filtering, wrapping style, etc.)
// - upload the imported image data to the OpenGL texture
// - don't forget to clean up
void initTextures (void) {
#ifdef EX7_4
	TextureData textureData[2];
	textureData[0] = loadTextureData("../../textures/trashbin.png");
	textureData[1] = loadTextureData("../../textures/ball.jpg");
	glGenTextures(1, &textureData[0].texture);
	glBindTexture(GL_TEXTURE_2D, textureData[0].texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData[0].width, textureData[0].height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData[0].data);
	textures["trashbin"] = textureData[0];

	glGenTextures(1, &textureData[1].texture);
	glBindTexture(GL_TEXTURE_2D, textureData[1].texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData[1].width, textureData[1].height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData[1].data);
	textures["ball"] = textureData[1];
#else
	Material mat;
	int index = 0;
	for (std::vector<MeshMaterial>::iterator it = objLoader->getMeshObj("optimus")->materials().begin();
		it != objLoader->getMeshObj("optimus")->materials().end(); it++, index++) {
		TextureData textureData;

		std::string str("../../textures/");
		str += it->texture_map;

		textureData = loadTextureData(str.c_str());
		glGenTextures(1, &textureData.texture);
		glBindTexture(GL_TEXTURE_2D, textureData.texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData.width, textureData.height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData.data);

		std::stringstream sstr("");
		sstr << "optimus_" << index;
		textures[sstr.str()] = textureData;
		std::cout << sstr.str() << std::endl;
	}

	index = 0;
	for (std::vector<MeshMaterial>::iterator it = objLoader->getMeshObj("megatron")->materials().begin();
		it != objLoader->getMeshObj("megatron")->materials().end(); it++, index++) {
		TextureData textureData;

		std::string str("../../textures/");
		str += it->texture_map;

		textureData = loadTextureData(str.c_str());
		glGenTextures(1, &textureData.texture);
		glBindTexture(GL_TEXTURE_2D, textureData.texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData.width, textureData.height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData.data);

		std::stringstream sstr("");
		sstr << "megatron_" << index;
		textures[sstr.str()] = textureData;
		std::cout << sstr.str() << std::endl;
	}
#endif
}

// TODO: load texture data from disk //
// - return imported data as 'TextureData' container
// - hint: use OpenCV or FreeImage to import a image file
TextureData loadTextureData(const char *textureFile) {
  TextureData textureData;
  IplImage* img = cvLoadImage(textureFile);

  if (!img){
	  std::cerr << "Unable to load texture!" << std::endl;
	  exit(1);
  }
  else {
	  textureData.data = new unsigned char[img->imageSize]();
	  std::copy(img->imageData, img->imageData + (img->imageSize - 1), textureData.data);
	  textureData.width = img->width;
	  textureData.height = img->height;
  }
  return textureData;
}

float quad[] = {
	-1.0f, 1.0f, 0.0f, 	// v0 - top left corner
	-1.0f, -1.0f, 0.0f,	// v1 - bottom left corner
	1.0f, 1.0f, 0.0f, 	// v2 - top right corner
	1.0f, -1.0f, 0.0f	// v3 - bottom right corner
};

GLuint VAO_BG, VBO_BG;

void initGradientBg() {
	glGenVertexArrays(1, &VAO_BG);
	glBindVertexArray(VAO_BG);

	// Create the Vertex Buffer Object for the full screen quad.

	glGenBuffers(1, &VBO_BG);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BG);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void drawGradientBg() {
	glDisable(GL_DEPTH_TEST);
	glUniform1i(uniformLocations["usebg"], 1);
	glUniform1f(uniformLocations["height"], (GLfloat)windowHeight);
	glBindVertexArray(VAO_BG);

	// 4 vertices with 2 floats per vertex = 8 elements total.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUniform1i(uniformLocations["usebg"], 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void initScene() {
  // TODO (7.4) : load trashbin and ball from disk and create renderable meshes //
#ifdef EX7_4
	objLoader->loadObjFile("../../meshes/trashbin.obj", "trashbin");
	objLoader->loadObjFile("../../meshes/ball.obj", "ball");
#else
	objLoader->loadObjFile("../../meshes/optimus.obj", "optimus");
	objLoader->loadObjFile("../../meshes/megatron.obj", "megatron");
#endif
	initGradientBg();
  // TODO (7.5): load Optimus Prime and Megatron from disk and create renderable meshes //
  
  // init materials //
  Material mat;
  mat.ambient_color = glm::vec3(1.0, 1.0, 1.0);
  mat.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_shininess = 50.0;
  materials.push_back(mat);
  
  mat.specular_shininess = 100.0;
  materials.push_back(mat);
  
  mat.ambient_color = glm::vec3(1.0, 0.9, 0.8);
  mat.diffuse_color = glm::vec3(1.0, 0.9, 0.8);
  mat.specular_shininess = 50.0;
  materials.push_back(mat);
  
  // save material count for later and select first material //
  materialCount = materials.size();
  materialIndex = 0;
  
  // init lights //

  LightSource light;
  light.ambient_color = glm::vec3(0.001, 0.001, 0.001);
  light.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
  light.specular_color = glm::vec3(1.0, 1.0, 1.0);
  
  light.position = glm::vec3(15, 15, 15);
  lights.push_back(light);
  
  light.position = glm::vec3(-15, 15, 15);
  lights.push_back(light);
  
  light.position = glm::vec3(15, 15, -15);
  lights.push_back(light);
  
  light.position = glm::vec3(-15, 15, -15);
  lights.push_back(light);

  // Todo (7.5) : estimate nicer lighting situation for Transformers //
#ifndef EX7_4
  lights.clear();
  light.position = glm::vec3(0.2, 1.25, 0.48);
  lights.push_back(light);
  light.position = glm::vec3(0, 1.0, 0.14);
  lights.push_back(light);
#endif
  
  // save light source count for later and select first light source //
  lightCount = lights.size();
}

void renderScene() {
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  
  //draw gradient bg
  drawGradientBg();

  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  //glUniformMatrix4fv(uniformLocations["view"], 1, false, glm::value_ptr(camera.getModelViewMat()));

  // TODO: upload the properties of the currently active light sources here //
  // - ambient, diffuse and specular color
  // - position
  // - use glm::value_ptr() to get a proper reference when uploading the values as a data vector //
  int shaderLightIdx = 0;

  for (unsigned int i = 0; i < lightCount; ++i) {
	  if (lights[i].enabled) {
		  std::stringstream sstr;
		  sstr << "light_" << shaderLightIdx;
		  UniformLocation_Light &light = uniformLocations_Lights[sstr.str()];
		  glUniform3fv(light.position, 1, glm::value_ptr(lights[i].position));
		  glUniform3fv(light.ambient_color, 1, glm::value_ptr(lights[i].ambient_color));
		  glUniform3fv(light.diffuse_color, 1, glm::value_ptr(lights[i].diffuse_color));
		  glUniform3fv(light.specular_color, 1, glm::value_ptr(lights[i].specular_color));

		  shaderLightIdx++;
	  }
  }
  // upload lights here....

  glUniform1i(uniformLocations["usedLightCount"], shaderLightIdx);
  
#ifdef EX7_4
  // upload the chosen material properties here //
  glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
  glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
  glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
  glUniform1f(uniformLocations["material.shininess"], materials[materialIndex].specular_shininess);
  
  // TODO: upload respective texture to first texture unit and render the actual scene //
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());

  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));

  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures["trashbin"].texture);
  glUniform1i(uniformLocations["awesomeTexture"], 0);
  objLoader->getMeshObj("trashbin")->render();

  glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(-5.0f, 0.0f, 0));
  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  glBindTexture(GL_TEXTURE_2D, textures["ball"].texture);
  glUniform1i(uniformLocations["awesomeTexture"], 0);
  objLoader->getMeshObj("ball")->render();

  glm_ModelViewMatrix.pop();
#else
  // render megatron
  std::vector<MeshMaterial> &m_meg = objLoader->getMeshObj("megatron")->materials();

  glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(m_meg[materialIndexMegatron].ambient_color));
  glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(m_meg[materialIndexMegatron].diffuse_color));
  glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(m_meg[materialIndexMegatron].specular_color));
  glUniform1f(uniformLocations["material.shininess"], m_meg[materialIndexMegatron].specular_shininess);

  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());

  glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(-0.6f, 0.0f, 0.0f)) * glm::rotate(65.0f, glm::vec3(0, 1, 0));

  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  glActiveTexture(GL_TEXTURE0);
  std::stringstream sstr_("");
  sstr_ << "megatron_" << materialIndexMegatron;

  glBindTexture(GL_TEXTURE_2D, textures[sstr_.str()].texture);
  glUniform1i(uniformLocations["awesomeTexure"], 0);
  objLoader->getMeshObj("megatron")->render();

  glm_ModelViewMatrix.pop();

  // render optimus
  std::vector<MeshMaterial> &m_opt = objLoader->getMeshObj("optimus")->materials();

  glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(m_opt[materialIndexOptimus].ambient_color));
  glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(m_opt[materialIndexOptimus].diffuse_color));
  glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(m_opt[materialIndexOptimus].specular_color));
  glUniform1f(uniformLocations["material.shininess"], (m_opt[materialIndexOptimus].specular_shininess));

  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());

  glm_ModelViewMatrix.top() *= glm::translate(glm::vec3(0.6f, 0.0f, 0.0f)) * glm::rotate(-65.0f, glm::vec3(0, 1, 0));

  glUniformMatrix4fv(uniformLocations["modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  glActiveTexture(GL_TEXTURE0);
  std::stringstream sstr("");
  sstr << "optimus_" << materialIndexOptimus;

  glBindTexture(GL_TEXTURE_2D, textures[sstr.str()].texture);
  glUniform1i(uniformLocations["awesomeTexure"], 0);
  objLoader->getMeshObj("optimus")->render();

  glm_ModelViewMatrix.pop();
#endif

  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
}

void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // set viewport dimensions //
  glViewport(0, 0, windowWidth, windowHeight);
  
  // get projection mat from camera controller //
  glm_ProjectionMatrix.top() = camera.getProjectionMat();
  // upload projection matrix //
  glUniformMatrix4fv(uniformLocations["projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
  
  // init scene graph by cloning the top entry, which can now be manipulated //
  // get modelview mat from camera controller //
  glm_ModelViewMatrix.top() = camera.getModelViewMat();
  
  // render scene //
  renderScene();
  
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

void keyboardEvent(unsigned char key, int x, int y) {
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

	case 'j': {
		// toggle optimus material
		int size = objLoader->getMeshObj("optimus")->materials().size();
		if (materialIndexOptimus < (size - 1))
			materialIndexOptimus++;
		else
			materialIndexOptimus = 0;
		break;
	}
	case 'k': {
		// toggle megatron material
		int size = objLoader->getMeshObj("megatron")->materials().size();
		if (materialIndexMegatron < (size - 1))
			materialIndexMegatron++;
		else
			materialIndexMegatron = 0;
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
    case 'z': {
      camera.setOpeningAngle(camera.getOpeningAngle() + 0.1f);
      break;
    }
    case 'h': {
      camera.setOpeningAngle(std::min(std::max(camera.getOpeningAngle() - 0.1f, 1.0f), 180.0f));
      break;
    }
    case 'r': {
      camera.setNear(std::min(camera.getNear() + 0.1f, camera.getFar() - 0.01f));
      break;
    }
    case 'f': {
      camera.setNear(std::max(camera.getNear() - 0.1f, 0.1f));
      break;
    }
    case 't': {
      camera.setFar(camera.getFar() + 0.1f);
      break;
    }
    case 'g': {
      camera.setFar(std::max(camera.getFar() - 0.1f, camera.getNear() + 0.01f));
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
      default : 
		  mouseState = CameraController::NO_BTN;
		  break;
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

