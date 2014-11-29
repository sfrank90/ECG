#include "Ex04.h"

// OpenGL and GLSL stuff //
void initGL();
void initShader();
bool enableShader();
void disableShader();
void deleteShader();
bool glError(char *msg);

GLuint loadShaderFile(const char* fileName, GLenum shaderType);
GLuint shaderProgram = 0;

// camera controls //
CameraController cameraView(0, 0, 1.0);			// camera looks frontal on scene, distance from origin = 1.0
CameraController sceneView(M_PI/4, M_PI/6, 4);  // second camera looks from above/side, distance from origin = 4.0

// canonical volume start orientation
glm::quat cv_Rotation = glm::quat(0.98, 0.21, 0.07, 0.0);
GLfloat cv_scale = 0.55;
GLfloat rotAngle = 0;
bool rotAnim	 = true;

// viewport //
GLint windowWidth	= 1736;
GLint windowHeight	= 512;
GLint guiWidth		= 200;
GLint numViews		= 3;
GLint widthview;
enum VIEW {CAMERA_VIEW, WORLD_VIEW, CV_VIEW};

glm::mat4 invertProjectionMat(const glm::mat4 &mat);
void renderCameraView();
void renderCameraSpaceVisualization();
void renderCanonicalVolumeVisualization();

// window controls //
void updateGL();
void idle();
void Terminate(void);
void setViewport(VIEW view);
void Reshape(int width, int height);
void keyboardEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// geometry //
void initScene();
void renderScene();
void deleteScene();
// cube representing camera frustum //
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
GLuint cubeIBO = 0;
// camera front side is rendered green
GLuint camSideVAO = 0;
GLuint camSideVBO = 0;
GLuint camSideIBO = 0;


//to find third view rotation
GLfloat thirdViewAngleX = -10.f;
GLfloat thirdViewAngleY = 58.f;
GLfloat thirdViewAngleZ = -20.f;

int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  glutInitWindowSize (windowWidth, windowHeight);
  glutInitWindowPosition (10, 10);
  glutCreateWindow("Exercise 04 - Camera control and projection");
  glutCreateMenu(NULL);
  widthview = (windowWidth - guiWidth) / numViews;
  std::cout << "Width of a single view = " << widthview << std::endl;

  glutDisplayFunc(updateGL);
  glutReshapeFunc(Reshape);
  glutIdleFunc(idle);
  atexit(Terminate);  // Called after glutMainLoop ends
  
  // configure Camera
  cameraView.setFar(1.5);
  cameraView.setNear(0.5);
  cameraView.setOpeningAngle(45.0);
  cameraView.setAspect(1.2);

  // Initialize the GUI
  TwInit(TW_OPENGL_CORE, NULL);
  TwWindowSize(windowWidth, windowHeight);
  TwBar * CameraGUI = TwNewBar("CameraGUI");
  TwDefine(" GLOBAL help='Exercise Sheet 04 - Camera controller and projection.' "); // Message added to the help bar.
  
  // set position and size of AntTweakBar
  char *setbarposition = new char[1000];
  sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
  std::cout << setbarposition << std::endl;
  TwDefine(setbarposition);
  delete[] setbarposition;

  // configure GUI editing bars
  TwSetParam(CameraGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
  TwAddVarRW(CameraGUI, "zNear", TW_TYPE_FLOAT, &cameraView.mNear, "step=0.01");
  TwAddVarRW(CameraGUI, "zFar", TW_TYPE_FLOAT, &cameraView.mFar, "step=0.01");
  TwAddVarRW(CameraGUI, "OpenAngle", TW_TYPE_FLOAT, &cameraView.mOpenAngle, "step=0.01");
  TwAddVarRW(CameraGUI, "Aspect", TW_TYPE_FLOAT, &cameraView.mAspect, "step=0.01");
  TwAddVarRW(CameraGUI, "Theta", TW_TYPE_FLOAT, &cameraView.mTheta, "step=0.01");
  TwAddVarRW(CameraGUI, "Phi", TW_TYPE_FLOAT, &cameraView.mPhi, "step=0.01");
  TwAddVarRW(CameraGUI, "CV Navigation", TW_TYPE_QUAT4F, &cv_Rotation, "showval=false open=true");
  TwAddVarRW(CameraGUI, "scale CV", TW_TYPE_FLOAT, &cv_scale, "step=0.01");
  TwAddVarRW(CameraGUI, "Auto-Rotation", TW_TYPE_BOOL8 , &rotAnim, "help='Auto-rotate canonical volume?'");
  TwAddVarRW(CameraGUI, "Angle", TW_TYPE_FLOAT, &rotAngle, "step=1.0");
  TwAddVarRW(CameraGUI, "AngleX", TW_TYPE_FLOAT, &thirdViewAngleX, "step=1.0");
  TwAddVarRW(CameraGUI, "AngleY", TW_TYPE_FLOAT, &thirdViewAngleY, "step=1.0");
  TwAddVarRW(CameraGUI, "AngleZ", TW_TYPE_FLOAT, &thirdViewAngleZ, "step=1.0");
  
  // redirect GLUT events to AntTweakBar
  glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
  glutMouseFunc(mouseEvent);
  glutMotionFunc(mouseMoveEvent);
  glutKeyboardFunc(keyboardEvent);
  glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
  TwGLUTModifiersFunc(glutGetModifiers);

  // initialize GL content
  glError("creating rendering context");
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  glError("rendering context created");
  
  // init cameras //
  sceneView.setFar(100);

  // init OpenGL //
  initGL();
  glError("GL initialized");
  
  // init matrix stacks //
  glm_ProjectionMatrix.push(glm::mat4(1.0));
  glm_ModelViewMatrix.push(glm::mat4(1.0));
  
  initShader();
  glError("shader initialized");

  initScene();
  glError("scene initialized");
  
  // start render loop //
  if (enableShader()) {
	glError("enableShader()");

    glutMainLoop();
	disableShader();

    // clean up allocated data //
    deleteScene();
    deleteShader();
  }
  
  return 0;
}

// Function called at exit
void Terminate(void)
{ 
    TwTerminate();
}

void setViewport(VIEW view) {
	// TODO : compute viewport size and position using enum View
	GLint x;
	switch (view) {
		case CAMERA_VIEW: 
			x = 0;
			break;
		case WORLD_VIEW: 
			x = widthview;
			break;
		case CV_VIEW: 
			x = 2 * widthview;
			break;
	}
	glViewport(x, 0, widthview, windowHeight);
}

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
	windowWidth = width;
	windowHeight = height;
	widthview = (windowWidth - guiWidth) / numViews;

    // Send the new window size to AntTweakBar
    TwWindowSize(windowWidth, windowHeight);

	// set position and size of AntTweakBar
	char *setbarposition = new char[1000];
	sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
	std::cout << setbarposition << std::endl;
	TwDefine(setbarposition);
	delete[] setbarposition;

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
  
  GLuint vertexShader = loadShaderFile("../shader/ndc.vert", GL_VERTEX_SHADER);
  if (vertexShader == 0) {
    std::cout << "(initShader) - Could not create vertex shader." << std::endl;
    deleteShader();
    return;
  }
  glError("loadShaderFile(GL_VERTEX_SHADER)");

  GLuint fragmentShader = loadShaderFile("../shader/ndc.frag", GL_FRAGMENT_SHADER);
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
  std::cout << "=================================" << std::endl;
  if (shaderType == GL_VERTEX_SHADER) {
	  std::cout << "VERTEX SHADER CODE " << std::endl;
  }
  else if (shaderType == GL_FRAGMENT_SHADER) {
	  std::cout << "FRAGMENT SHADER CODE " << std::endl;
  }
  std::cout << shaderSrc << std::endl;
  std::cout << "=================================\n\n\n" << std::endl;

  if (shaderSrc == NULL) return 0;
  // pass source code to new shader object //
  glShaderSource(shader, 1, &shaderSrc, NULL);

  // compile shader //
  glCompileShader(shader);
  
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


// compute inverse of non-affine matrices
glm::mat4 invertProjectionMat(const glm::mat4 &mat) {
  
  // This method returns the inverse of a matrix, that is not affine (like OpenGLs projection matrix).
  // Input parameter  : Reference to the matrix to be inverted.
  // Output parameter : Inverse of provided matrix.

  glm::mat4 inv(0);
  inv[0][0] = 1 / mat[0][0];
  inv[1][1] = 1 / mat[1][1];
  inv[2][2] = mat[3][3];
  inv[2][3] = 1 / mat[3][2];
  inv[3][2] = mat[2][3];
  inv[3][3] = mat[2][2] / mat[3][2];
  return inv;
}

ObjLoader objLoader;

void initScene() {
  
  objLoader.loadObjFile("../meshes/armadillo.obj", "armadillo");
  objLoader.loadObjFile("../meshes/bunny.obj", "bunny");
  // TODO : load camera.obj from disk
  objLoader.loadObjFile("../meshes/camera.obj", "camera");

  // init frustum cube //
  GLfloat frustrumVertices[24] = {-1,-1,-1,
				   1,-1,-1,
				   1, 1,-1,
				  -1, 1,-1,
				  -1,-1, 1,
				   1,-1, 1,
				   1, 1, 1,
				  -1, 1, 1};
  // define indices for cube
  GLuint frustumIndices[24] = {
				   0, 1,
			       1, 2,
			       2, 3,
			       3, 0,
			       4, 5,
			       5, 6,
			       6, 7,
			       7, 4,
			       0, 4,
			       1, 5,
			       2, 6,
			       3, 7};

  // TODO : create and fill buffers for cube, upload data
  //	- cubeVAO
  if (cubeVAO == 0) {
	  glGenVertexArrays(1, &cubeVAO);
  }
  glBindVertexArray(cubeVAO);
  
  //	- cubeVBO
  if (cubeVBO == 0) {
	  glGenBuffers(1, &cubeVBO);
  }
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), &frustrumVertices[0], GL_STATIC_DRAW); //24 are specified in frustrumVertices
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  //shader uses normals for color -> set to the vertices to see the frustum!
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(1);

  //	- cubeIBO
  if (cubeIBO == 0) {
	  glGenBuffers(1, &cubeIBO);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLuint), &frustumIndices[0], GL_STATIC_DRAW); //24 are specified in frustumIndices
  
  //	- unbind vertex array at the end
  glBindVertexArray(0);

  // TODO : define two triangles on camera front side (see figure in the exercise PDF),
  //	- create index array defining the triangles (array "camSideIndices")
  //	- create and fill buffers for triangles
  //		- camSideVAO
  //		- camSideVBO (use frustumVertices array also used for cube)
  //		- camSideIBO
  //	- unbind vertex array at the end
  GLuint camSideIndices[6] = {
	  0, 1, 3,
	  1, 2, 3
	  };

  //camSideVAO
  if (camSideVAO == 0) {
	  glGenVertexArrays(1, &camSideVAO);
  }
  glBindVertexArray(camSideVAO);

  //camSideVBO
  if (camSideVBO == 0) {
	  glGenBuffers(1, &camSideVBO);
  }
  glBindBuffer(GL_ARRAY_BUFFER, camSideVBO);
  glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), &frustrumVertices[0], GL_STATIC_DRAW); //24 are specified in frustrumVertices
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  //camSideIBO
  if (camSideIBO == 0) {
	  glGenBuffers(1, &camSideIBO);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, camSideIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), &camSideIndices[0], GL_STATIC_DRAW); //24 are specified in camSideIndices
  
  //	- unbind vertex array at the end
  glBindVertexArray(0);
}

void deleteScene() {
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteBuffers(1, &cubeIBO);
  glDeleteVertexArrays(1, &camSideVAO);
  glDeleteBuffers(1, &camSideVBO);
  glDeleteBuffers(1, &camSideIBO);
}

void renderScene() {
  // init scene graph by cloning the top entry, which can now be manipulated //
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());//1

  GLfloat rot_armadillo_y[] = {45.f, -45.f, 135.f, -135.f};
  GLfloat rot_bunny_y[] = {85.f, -5.f, 175.f, -95.f};

  for (int y = -1; y <= 1; y+=2) {
    for (int x = -1; x <= 1; x+=2) {
	  // armadillo
      glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());//2
      glm_ModelViewMatrix.top() *= glm::translate(glm::vec3((GLfloat)x * 0.15, 0.0, (GLfloat)y * 0.15));
	  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1));
	  glm_ModelViewMatrix.top() *= glm::rotate(rot_armadillo_y[glm::max(y,0)*2+glm::max(x,0)], glm::vec3(0,1,0));
	  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	  objLoader.getMeshObj("armadillo")->render();
	  glm_ModelViewMatrix.pop();//1
	  // bunny
	  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());//2
	  glm_ModelViewMatrix.top() *= glm::translate(glm::vec3((GLfloat)x * 0.35, 0.0, (GLfloat)y * 0.35));
	  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1));
	  glm_ModelViewMatrix.top() *= glm::rotate(rot_bunny_y[glm::max(y,0)*2+glm::max(x,0)], glm::vec3(0,1,0));
	  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	  objLoader.getMeshObj("bunny")->render();
	  glm_ModelViewMatrix.pop();//1
    }
  }
  
  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();//0
}

// camera perspective
void renderCameraView() {

	// TODO : set viewport to left third of the window using setViewport(...) //
	setViewport(VIEW::CAMERA_VIEW);
	// TODO : set the correct projection matrix to uniform variable "projection"
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(cameraView.getProjectionMat()));
	// TODO : set post transformation to identity in shader (uniform variable "cv_transform")
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(glm::mat4(1.0f)));
	// TODO : get model view matrix from the camera and set it on top of model view stack
	glm_ModelViewMatrix.push(cameraView.getModelViewMat());//1
	// TODO : render scene
	renderScene();
	// restore scene graph to previous state //
	glm_ModelViewMatrix.pop();//0
}

// camera frustum in world space
void renderCameraSpaceVisualization() {
	
	// TODO: set viewport to middle third of the window using setViewport(...) //
	setViewport(VIEW::WORLD_VIEW);
	// TODO : set post transformation to identity in shader (uniform variable "cv_transform")
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(glm::mat4(1.0f)));
	// TODO : set the correct projection matrix to uniform variable "projection"
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(sceneView.getProjectionMat()));
	// TODO : get model view matrix from the second camera and set it on top of model view stack
	glm_ModelViewMatrix.push(sceneView.getModelViewMat());//1
	// TODO : render the scene
	renderScene();
	// TODO : render the camera model
	//	- compute and set the correct model view matrix containing the inversed model view of the first camera
	//	  You should use the function glm::affineInverse() for this task.
	//  - scale the model down uniformly to 0.1 so that the model has an apropriate dimension
	//	- render the model
	// init scene graph by cloning the top entry, which can now be manipulated //
	glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());//2
	glm_ModelViewMatrix.top() *= glm::affineInverse(cameraView.getModelViewMat());
	glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.1, 0.1, 0.1));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	objLoader.getMeshObj("camera")->render();
	glm_ModelViewMatrix.pop();//1

	// TODO : render the camera frustum
	//	- Compute and set the correct model view matrix, containing the inversed
	//	  model view matrix and inversed projection matrix of the first camera.
	//	  Since a projection matrix represents no affine transformation, use the
	//	  function invertProjectionMat() for computation.
	//	- render the frustum cube
	// init scene graph by cloning the top entry, which can now be manipulated //
	glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());//2
	glm_ModelViewMatrix.top() *= glm::affineInverse(cameraView.getModelViewMat());
	glm_ModelViewMatrix.top() *= invertProjectionMat(cameraView.getProjectionMat());
	
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	if (cubeVAO != 0) {
		glBindVertexArray(cubeVAO);
		glError("Bind cubeVAO: ");
		glDrawElements(
			GL_LINES,      // mode
			24,   // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			);

		glError("Drawing Lines");
		// unbind active buffers //
		glBindVertexArray(0);
	}
	
	// restore scene graph to previous state //
	glm_ModelViewMatrix.pop();//1

	// restore scene graph to previous state //
	glm_ModelViewMatrix.pop();//0
}

// projected camera frustum (canonical view)
void renderCanonicalVolumeVisualization() {

	// TODO: set viewport to right third of the window using setViewport(...) //
	setViewport(VIEW::CV_VIEW);
	glm_ModelViewMatrix.push(cameraView.getModelViewMat());//1

	// TODO : VISUALIZE RENDERED SCENE IN CANONICAL VOLUME
	//	- set projection matrix
	//	- set the additional transformation (scaling and rotation)
	//	  to the value "cv_transform" in the shader to "see" the 
	//	  normalized device space
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(cameraView.getProjectionMat()));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));

	glm::mat4 cv = glm::mat4(1.0f);
	
	cv *= glm::scale(glm::vec3(0.5, 0.5, 0.5));
	cv *= glm::rotate(thirdViewAngleY, glm::vec3(0, 1, 0));
	cv *= glm::rotate(thirdViewAngleX, glm::vec3(1, 0, 0));
	cv *= glm::rotate(thirdViewAngleZ, glm::vec3(0, 0, 1));
	// TODO : send "cv_transform" to the shader program
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cv_transform"), 1, false, glm::value_ptr(cv));



	// TODO: render the scene with manually defined clipping planes
	// Clipping distance depends on scaling of visualization (cv_scale)
	//	- This information has to be set in the shader ("clip_plane_distance").
	//	- enable manual clipping via clipping distance in OpenGL (GL_CLIP_DISTANCE0)
	//	- set the model view matrix saved in the camera instance on top of the stack
	//  - render scene
	glEnable(GL_CLIP_DISTANCE0);
	glUniform1f(glGetUniformLocation(shaderProgram, "clip_plane_distance"), cv_scale);


	renderScene();

	// disable clipping distance again
	glDisable(GL_CLIP_DISTANCE0);

	// VISUALIZE CANONICAL VOLUME LIMITS
	
	// TODO : render frustum
	//		- set projection to CV transformation only, since CV "is projected" already
	//		- set modelview to identity
	//		- draw the frustum as lines
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, false, glm::value_ptr(glm::mat4(1.0f)));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(glm::mat4(1.0f)));

	if (cubeVAO != 0) {
		// TODO: bind VAO //
		glBindVertexArray(cubeVAO);
		glError("Bind cubeVAO: ");
		glDrawElements(
			GL_LINES,      // mode
			24,   // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			);

		glError("Drawing Lines");
		// unbind active buffers //
		glBindVertexArray(0);
	}

	// TODO : draw two triangles defining the camera side
	//	- active color override in the fragment shader by setting "use_override_color" to 1
	//	- set uniform "override_color" to (green = 0,1,0) in the shader program
	//	- render the triangles
	//	- set "use_override_color" back to 0
	glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), 1);
	glUniform3f(glGetUniformLocation(shaderProgram, "override_color"), 0.f, 1.f, 0.f);
	glError("override_color");

	if (camSideVAO != 0) {
		// TODO: bind VAO //
		glBindVertexArray(camSideVAO);
		glError("Bind camSideVAO: ");
		// TODO: render data as triangles //
		glDrawElements(
			GL_TRIANGLES,      // mode
			6,   // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			);

		glError("Drawing green triangles");
		// unbind active buffers //
		glBindVertexArray(0);
	}

	glUniform1i(glGetUniformLocation(shaderProgram, "use_override_color"), 0);
	// TODO : render camera
	//	- scale camera model to a sensible value
	//  - decide where to put the camera model (think about the canonical volume in OpenGL!)
	//	- draw the camera
	glm::mat4 cameraTrans = glm::mat4(1.0f);
	cameraTrans *= glm::translate(0.f, 0.f, -1.1f);
	cameraTrans *= glm::rotate(180.f, glm::vec3(0, 1, 0));	
	cameraTrans *= glm::scale(glm::vec3(0.1, 0.1, 0.1));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, false, glm::value_ptr(cameraTrans));
	objLoader.getMeshObj("camera")->render();
	

	glm_ModelViewMatrix.push(sceneView.getModelViewMat());//0
}

void updateGL() {

	// clear buffer, enable transparency
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );

	// Draw views //
	renderCameraView();
	renderCameraSpaceVisualization();
	renderCanonicalVolumeVisualization();
	  
	// increment rotation angle //
	if (rotAnim == true)
	rotAngle += 0.2f;
	if (rotAngle > 360.0f) rotAngle -= 360.0f;
    
	// Draw GUI //
	TwDraw();

	// swap renderbuffers for smooth rendering //
	glutSwapBuffers();
}

void idle() {
  glutPostRedisplay();
}

void keyboardEvent(unsigned char key, int x, int y) {

	if (TwEventKeyboardGLUT(key, x, y) != 1) { // if GUI has not responded to event

		// check interaction in views
		CameraController *camera = 0;
		// camera view
		if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
			camera = &cameraView;
		}
		// world view
		else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
			camera = &sceneView;
		}
		// CV view not interactive at the moment
		// ...

		// move camera in respective view
		if (camera != 0) {
			switch (key) {
				case 'x':
				case 27 : {
				  exit(0);
				  break;
				}
				case 'w': {
				  // move forward //
				  camera->move(CameraController::MOVE_FORWARD);
				  break;
				}
				case 's': {
				  // move backward //
				  camera->move(CameraController::MOVE_BACKWARD);
				  break;
				}
				case 'a': {
				  // move left //
				  camera->move(CameraController::MOVE_LEFT);
				  break;
				}
				case 'd': {
				  // move right //
				  camera->move(CameraController::MOVE_RIGHT);
				  break;
				}
				case 'r': {
					// move up //
					camera->move(CameraController::MOVE_UP);
				break;
				}
				case 'f': {
					// move down //
					camera->move(CameraController::MOVE_DOWN);
				break;
				}
			}
		}
	}

	glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y) {

  // use event in GUI
  if (TwEventMouseButtonGLUT(button, state, x, y) != 1) { // if GUI has not responded to event

	  // check interaction in views
	  CameraController *camera = 0;
	  // camera view
	  if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
		  camera = &cameraView;
	  }
	  // world view
	  else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
		camera = &sceneView;
	  }

	  // CV view not interactive at the moment
	  // ...

	  // move camera in respective view
	  if (camera != 0) {
	  
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
		  camera->updateMouseBtn(mouseState, x, y);

	  }
  }
    
  glutPostRedisplay();
}

void mouseMoveEvent(int x, int y) {

  // use event in GUI
  if (TwEventMouseMotionGLUT(x, y) != 1) { // if GUI has not responded to event
	  
	  // check interaction in views
	  CameraController *camera = 0;
	  // camera view
	  if ((x >= 0) && (x < (widthview * (WORLD_VIEW)))) {
		  camera = &cameraView;
	  }
	  // world view
	  else if ((x >= (widthview * WORLD_VIEW)) && (x < (widthview * (WORLD_VIEW+1)))) {
		camera = &sceneView;
	  }
	  // CV view not interactive at the moment
	  // ...

	  // move camera in respective view
	  if (camera != 0) {
		camera->updateMousePos(x, y);
	  }
  }

  glutPostRedisplay();
}

