#include "Ex02.h"

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
char* loadShaderSource(const char* fileName);
GLuint loadShaderFile(const char* fileName, GLenum shaderType);
GLuint shaderProgram = 0;
GLint uniform_projectionMatrix;
GLint uniform_modelViewMatrix;

// window controls //
void updateGL();
void idle();
void keyboardEvent(unsigned char key, int x, int y);

// geometry //
GLuint bunnyVAO;
GLuint bunnyVBO;
GLuint bunnyIBO;

void readModel();
void initScene();
void deleteScene();
void renderScene();

GLfloat vertexList[NUM_POINTS * 6];
// view matrices //
glm::mat4 projectionMatrix = glm::mat4(2.414214, 0.000000, 0.000000, 0.000000, 0.000000, 2.414214, 0.000000, 0.000000, 0.000000, 0.000000, -1.002002, -1.000000, 0.000000, 0.000000, -0.020020, 0.000000);
glm::mat4 modelViewMatrix = glm::mat4(0.707107, -0.408248, 0.577350, 0.000000, 0.000000, 0.816497, 0.577350, 0.000000, -0.707107, -0.408248, 0.577350, 0.000000, 0.025249, -0.085015, -0.391099, 1.000000);


int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutInitWindowSize (512, 512);
  glutInitWindowPosition (100, 100);
  glutCreateWindow("Exercise 02 - Play with the bunny");
  
  glutDisplayFunc(updateGL);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboardEvent);
  
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  
  // init stuff //
  initGL();
  initShader();
  initScene();
  
  // start render loop //
  if (enableShader()) {
    glutMainLoop();
    disableShader();
    
    // clean up allocated data //
    deleteScene();
    deleteShader();
  }
  
  return 0;
}

void initGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
}

void initShader() {
  // TODO:
  //  load the vertex and fragment shader from source files attach them to a shader program
  //  - load the file "shader/simple.vert" as GL_VERTEX_SHADER
  //  - load the file "shader/simple.frag" as GL_FRAGMENT_SHADER
  //  - create a shader program "shaderProgram" (global member)
  //  - attach both shaders to this shader program
  //  - link the shader program
  
  //create new shader program "shaderProgram" //
	std::cout << "Linking program" << std::endl;
	shaderProgram = glCreateProgram();

  // check if operation failed //
  if (shaderProgram == 0) {
    std::cout << "(initShader) - Failed creating shader program." << std::endl;
    return;
  }
  
  //load vertex shader source //
  GLuint vertShader = loadShaderFile("../../shader/simple.vert", GL_VERTEX_SHADER);


  //load fragment shader source //
  GLuint fragShader = loadShaderFile("../../shader/simple.frag", GL_FRAGMENT_SHADER);
  
  // successfully loaded and compiled shaders -> attach them to program //
  //attach shaders to "shaderProgram" //
  glAttachShader(shaderProgram, vertShader);
  glAttachShader(shaderProgram, fragShader);

  // link shader program //
  glLinkProgram(shaderProgram);
  
  // get log //
  int logMaxLength;
  glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logMaxLength);
  char* log= new char[logMaxLength];
  int logLength = 0;
  glGetShaderInfoLog(shaderProgram, logMaxLength, &logLength, log);
  if (logLength > 0) {
    std::cout << "(initShader) - Linker log:\n------------------\n" << log << "\n------------------" << std::endl;
  }
  
  //cleanup
  delete[] log;

  // set address of fragment color output //
  glBindFragDataLocation(shaderProgram, 0, "color");
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
//the calles of this function is responsible to delete the char array later!
char* loadShaderSource(const char* fileName) {
  char *shaderSource = NULL;
  
  // TODO:
  //  import source code from a given file
  //  - open file
  //  - read in file into char array
  //  - close file and return array

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
  // TODO:
  //  create a shader program using code from a given file
  //  - create new shader of appropriate shader type (defined by 'shaderType')
  //  - load shader source from file
  //  - load shader source into shader and compile
  //  - (check for compilation errors (shader info log) )
  //  - return compiled shader
  
  // Create the Shader object
  GLuint shader = glCreateShader(shaderType);
  
  
  // check if operation failed //
  if (shader == 0) {
    std::cout << "(loadShaderFile) - Could not create shader." << std::endl;
    return 0;
  }
  
  //load source code from file //
  char* shaderSource = loadShaderSource(fileName);

  //pass source code to new shader object //
  glShaderSource(shader, 1, &shaderSource, NULL);

  //compile shader //
  glCompileShader(shader);

  // log compile messages, if any //
  int logMaxLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logMaxLength);
  char *log = new char[logMaxLength];
  int logLength = 0;
  glGetShaderInfoLog(shader, logMaxLength, &logLength, log);
  if (logLength > 0) {
    std::cout << "(loadShaderFile) - Compiler log:\n------------------\n" << log << "\n------------------" << std::endl;
  }
  delete[] log;
  
  // return compiled shader (may have compiled WITH errors) //
  return shader;
}

void readModel() {

}

void initScene() {  
  // TODO: import data from bunny.h and concatenate vertex and normal data //
	for (int i = 0; i < 3 * NUM_POINTS; i++) {
		vertexList[i] = bunny[i];
		vertexList[3 * NUM_POINTS + i] = normals[i];
	}
  // TODO: init and bind a VAO (vertex array object) //
	glGenVertexArrays(1, &bunnyVAO);
	glBindVertexArray(bunnyVAO);
  
  // TODO: init and bind a VBO (vertex buffer object) //
	glGenBuffers(1, &bunnyVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);
  
  // TODO: copy data into the VBO //
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_POINTS*sizeof(GLfloat), &vertexList[0], GL_STATIC_DRAW);
	// => Vertices
	glVertexAttribPointer(
						0,			// attribute
						3,			// size
						GL_FLOAT,	// type
						GL_FALSE,	// normalized?
						0,			// stride
						(void*)0	// array buffer offset
					);
	
	// => Normals
	glVertexAttribPointer(
						1,                                // attribute
						3,                                // size
						GL_FLOAT,                         // type
						GL_FALSE,                         // normalized?
						0,                                  // stride
						(void*)(3 * NUM_POINTS*sizeof(GLfloat))                 // array buffer offset
					);

	glEnableVertexAttribArray(1);
  
  // TODO: init and bind a IBO (index buffer object) //
	glGenBuffers(1, &bunnyIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIBO);
  
  // TODO: copy data into the IBO //
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * NUM_TRIANGLES * sizeof(unsigned int), &triangles[0], GL_STATIC_DRAW);
  
  // unbind active buffers //
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void deleteScene() {
  if (bunnyIBO != 0) glDeleteBuffers(1, &bunnyIBO);
  if (bunnyVBO != 0) glDeleteBuffers(1, &bunnyVBO);
  if (bunnyVAO != 0) glDeleteVertexArrays(1, &bunnyVAO);
}

void renderScene() {
  if (bunnyVAO != 0) {
    // TODO: bind VAO //
	  glBindVertexArray(bunnyVAO);
	  glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIBO);
    
    // TODO: render data as triangles //
	  glDrawElements(
			  GL_TRIANGLES,      // mode
			  3 * NUM_TRIANGLES,   // count
			  GL_UNSIGNED_INT,   // type
			  (void*)0           // element array buffer offset
		  );
    // unbind active buffers //
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}

GLfloat rotAngle = 0;
void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(modelViewMatrix));

  // now the scene will be rendered //
  renderScene();
  
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
