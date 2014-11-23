#include "Ex03.h"

#define PATHCORRECTION

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
char* loadShaderSource(const char* fileName);
bool glError(char *msg);

GLuint loadShaderFile(const char* fileName, GLenum shaderType);
GLuint shaderProgram = 0;
GLint uniform_projectionMatrix;
GLint uniform_modelViewMatrix;

// window controls //
void updateGL();
void idle();
void keyboardEvent(unsigned char key, int x, int y);

// geometry //
void initScene();
void initSceneGraph();

int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutInitWindowSize (512, 512);
  glutInitWindowPosition (100, 100);
  glutCreateWindow("Exercise 03 - More creatures ... feeding time!");
  
  glutDisplayFunc(updateGL);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboardEvent);
  
  glError("creating rendering context");
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  glError("rendering context created");
  
  // init stuff //
  initGL();
  glError("GL initialized");
  
  int glVersion[2] = {0, 0}; 
  glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); 
  glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

  std::cout << "OpenGL Major Version: " << glVersion[0] << std::endl;
  std::cout << "OpenGL Minor Version: " << glVersion[1] << std::endl;

  // init matrix stacks //
  glm_ProjectionMatrix.push(glm::mat4(2.414214, 0.000000, 0.000000, 0.000000, 0.000000, 2.414214, 0.000000, 0.000000, 0.000000, 0.000000, -1.002002, -1.000000, 0.000000, 0.000000, -0.020020, 0.000000));
  glm_ModelViewMatrix.push(glm::mat4(0.707107, -0.408248, 0.577350, 0.000000, 0.000000, 0.816497, 0.577350, 0.000000, -0.707107, -0.408248, 0.577350, 0.000000, 0.000000, 0.000000, -1.732051, 1.000000));
  
  initShader();
  glError("shader initialized");

  initScene();
  glError("scene initialized");
  
  //create all the transform matrices
  initSceneGraph();

  // start render loop //
  if (enableShader()) {
	glError("enableShader()");

    glutMainLoop();

    disableShader();
  }
  
  std::cin.get();

  return 0;
}

bool glError(char *msg) {
	// check OpenGL error
    GLenum err;
	bool err_occured = false;
    while ((err = glGetError()) != GL_NO_ERROR) {
		err_occured = true;
		std::cerr << "OpenGL error ( " << msg << " ) : " << err << std::endl;
    }
	return err_occured;
}

void initGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

void initShader() {

  glError("begin initShader()");

  shaderProgram = glCreateProgram();
  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
  glError("glCreateProgram()");

#ifdef PATHCORRECTION
  GLuint vertexShader = loadShaderFile("../shader/simple.vert", GL_VERTEX_SHADER);
#else
  GLuint vertexShader = loadShaderFile("../../shader/simple.vert", GL_VERTEX_SHADER);
#endif
  if (vertexShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }

#ifdef PATHCORRECTION
  GLuint fragmentShader = loadShaderFile("../shader/simple.frag", GL_FRAGMENT_SHADER);
#else
  GLuint fragmentShader = loadShaderFile("../../shader/simple.frag", GL_FRAGMENT_SHADER);
#endif  
  if (fragmentShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  glError("loadShaderFile(GL_FRAGMENT_SHADER)");
  
  // successfully loaded and compiled shaders -> attach them to program //
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glError("glAttachShader()");
  
  // mark shaders for deletion after clean up (they will be deleted, when detached from all shader programs) //
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glError("glDeleteShader()");
  
  // link shader program //
  glLinkProgram(shaderProgram);
  glError("glLinkProgram()");

  
  // get log //
  GLint status;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
  glError("glGetProgramiv()");

  if (status == GL_FALSE)
  {
	GLint infoLogLength;
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

	GLchar* strInfoLog = new GLchar[infoLogLength + 1];
	glGetShaderInfoLog(shaderProgram, infoLogLength, NULL, strInfoLog);
	glError("glGetShaderInfoLog");

	std::cout << "(initShader) - Linker log:\n------------------\n" << strInfoLog << "\n------------------" << std::endl;
	delete[] strInfoLog;
	glError("strInfoLog()");
  }
  
  // set address of fragment color output //
  glBindFragDataLocation(shaderProgram, 0, "color");
  glError("glBindFragDataLocation()");
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
  glError("deleteShader()");
}

// load and compile shader code //
char* loadShaderSource(const char* fileName) {
	char *shaderSource = NULL;

	std::stringstream sstrm;
	std::ifstream fileStream(fileName, std::ios::in);
	if (!fileStream.good()) {
		std::cout << "Could not load \"" << fileName << "\"" << std::endl;
		return 0;
	}
	sstrm << fileStream.rdbuf();
	sstrm.seekg(0, std::ios::end);
	int size = sstrm.tellg();
	sstrm.seekg(0, std::ios::beg);

	shaderSource = new char[size + 1];

	strcpy(shaderSource, sstrm.str().c_str());
	shaderSource[size] = '\0';

	fileStream.close();
  return shaderSource;
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
  const char* shaderSrc = loadShaderSource(fileName);
  if (shaderSrc == NULL) return 0;
  // pass source code to new shader object //
  glShaderSource(shader, 1, (const char**)&shaderSrc, NULL);
  
  // compile shader //
  glCompileShader(shader);
  delete[] shaderSrc;
  GLint shaderStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderStatus);

  if (shaderStatus == GL_TRUE){
	  std::cout << "shader compiled successfully" << std::endl;
  }
  else{
	  std::cout << "shader not compiled successfully!!!" << std::endl;
  }

  glError("glCompileShader\n");
  
  // log compile messages, if any //
  GLint logMaxLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logMaxLength);
  GLchar *log = new GLchar[logMaxLength+1];
  GLint logLength = 0;
  glGetShaderInfoLog(shader, logMaxLength, &logLength, log);
  if (logLength > 0) {
    std::cout << "(loadShaderFile) - Compiler log:\n------------------\n" << log << "\n------------------" << std::endl;
  }
  delete[] log;
  
  // return compiled shader (may have compiled WITH errors) //
  return shader;
}

ObjLoader objLoader;

void initScene() {
  // load bunny.obj and armadillo.obj from disk and create renderable MeshObj //
#ifdef PATHCORRECTION
  objLoader.loadObjFile("../meshes/armadillo.obj", "armadillo");
  objLoader.loadObjFile("../meshes/bunny.obj", "bunny");
#else
  objLoader.loadObjFile("../../meshes/armadillo.obj", "armadillo");
  objLoader.loadObjFile("../../meshes/bunny.obj", "bunny");
#endif
}

//Transform matrices for bunnies and armadillos
std::vector<glm::mat4> transformArmadillos;
std::vector<glm::mat4> transformBunnies;

void initSceneGraph(){
	//transformations that are needed for more than one object
	glm::mat4 scaling = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));

	glm::mat4 translateArma = glm::translate(0.15f, 0.25f, 0.15f);
	glm::mat4 translateBunny = glm::translate(0.35f, 0.25f, 0.35f);

	glm::mat4 rotation90 = glm::rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation180 = glm::rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation225 = glm::rotate(225.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotation270 = glm::rotate(270.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	//transformations for the armadillos
	transformArmadillos.push_back(translateArma * scaling * rotation225);
	transformArmadillos.push_back(rotation90 * translateArma * scaling * rotation225);
	transformArmadillos.push_back(rotation180 * translateArma * scaling * rotation225);
	transformArmadillos.push_back(rotation270 * translateArma * scaling * rotation225);

	//transformations for the bunnies
	transformBunnies.push_back(translateBunny * scaling * rotation270);
	transformBunnies.push_back(rotation90 * translateBunny * scaling * rotation270);
	transformBunnies.push_back(rotation180 * translateBunny * scaling * rotation270);
	transformBunnies.push_back(rotation270 * translateBunny * scaling * rotation270);
}

GLfloat rotAngle = 0;
void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // projection matrix stays the same //
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));
  
  // init scene graph by cloning the top entry, which can now be manipulated //
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  
  // TODO: create a rotating grid of Stanford models (2 x 2 grid)
  //  - render one pair of bunny and armadillo in every quadrant of the grid (-> obj files loaded in initScene() already)
  //
  //    sketch: notation: B = bunny, A = armadillo
  //
  //          B       B
  //            A | A   
  //          ----------         
  //            A | A  
  //          B   |   B
  //
  //  - rotate the scene clockwise about 'rotAngle'
  //  - rotate the armadillos and bunnies so that each pair "looks" at each other (compare to image in pdf)
  //  - use glm_ModelViewMatrix.push(...) and glm_ModelViewMatrix.pop()   
  //  - apply new transformations by using: glm_ModelViewMatrix.top() *= glm::some_transformation(...);
  //  - right before rendering an object, upload the current state of the modelView matrix stack:
  //    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
  
  //camera rotation for one frame is the same for all objects
  //applying it to the modelview
  glm::mat4 rotation;
  rotation = glm::rotate(rotation, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
  glm_ModelViewMatrix.top() *= rotation;

  MeshObj* objArma = objLoader.getMeshObj("armadillo");
  MeshObj* objBunny = objLoader.getMeshObj("bunny");

  //Drawing 4 bunnies and 4 armadillos
  for (int i = 0; i < transformArmadillos.size() && i < transformBunnies.size(); i++)
  {
	  //first draw the armadillo...
	  //copying the modelview
	  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());

	  //applying transformation
	  glm_ModelViewMatrix.top() *= transformArmadillos[i];

	  //bring new modelview to gpu and then render the armadillo
	  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	  objArma->render();

	  //recovering the modelview
	  glm_ModelViewMatrix.pop();

	  //...then the bunny
	  //copying the modelview
	  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());

	  //applying transformation
	  glm_ModelViewMatrix.top() *= transformBunnies[i];

	  //bring new modelview to gpu and then render the bunny
	  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	  objBunny->render();

	  //recovering the modelview
	  glm_ModelViewMatrix.pop();
  }
  
  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
  
  // increment rotation angle //
  rotAngle += 0.1f;
  if (rotAngle > 360.0f) rotAngle -= 360.0f;
  
  // swap renderbuffers for smooth rendering //
  glutSwapBuffers();
}

void idle() {
  glutPostRedisplay();
}

void keyboardEvent(unsigned char key, int x, int y) {
  if (key == 'x' || key == 27) {
    exit(0);
  }
  glutPostRedisplay();
}
