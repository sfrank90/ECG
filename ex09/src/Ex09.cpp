#include "Ex09.h"

// OpenGL and GLSL stuff //
void initGL();
void initShader();


// shader program //
GLuint simpleShaderProg = 0;
// TODO: define two shader programs for hair rendering (lines and triangles)
GLuint hairLineProg = 0;
GLuint hairTrisProg = 0;

// map storing uniform locations of our shader program //
std::map<std::string, GLint> uniformLocations;


// matrix stacks //
std::stack<glm::mat4> glm_ProjectionMatrix; 
std::stack<glm::mat4> glm_ModelViewMatrix; 

// window controls //
void updateBar();
void updateGL();
void idle();
void close();
void Reshape(int width, int height);

// keyboard and mouse //
void keyDownEvent(unsigned char key, int x, int y);
void keyUpEvent(unsigned char key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void mouseMoveEvent(int x, int y);

// camera controls //
CameraController camera(0, 0, 1.0);

// viewport //
TwBar * CameraGUI;
GLint windowWidth, windowHeight;
GLint guiWidth;

// OBJ import //
ObjLoader *objLoader;

// materials and lighting //
unsigned int materialIndex;
unsigned int materialCount;
std::vector<Material> materials;
Material *currentMaterial;

unsigned int lightIndex;
unsigned int lightCount;
std::vector<LightSource> lights;
LightSource *currentLight;

// scene controls //
void initScene();
void renderScene();

bool renderModel = true;
bool renderHair = true;
bool renderHairLines = false;

// TODO: define variables for hair data (hairLength, gravity, hairSegments, hairWidth)
GLfloat hairLength = 0.01;
GLfloat gravity = 0.1;
GLuint hairSegments = 8;
GLfloat hairWidth = 0.5;

int main (int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitContextVersion(3,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  windowWidth = 812;
  windowHeight = 512;
  guiWidth = 300;

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Exercise 09 - Geometry Shader");
  
  glutDisplayFunc(updateGL);
  glutReshapeFunc(Reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyDownEvent);
  glutKeyboardUpFunc(keyUpEvent); 
  glutMouseFunc(mouseEvent);
  glutMotionFunc(mouseMoveEvent);
  glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
  TwGLUTModifiersFunc(glutGetModifiers);

  // Initialize the GUI
  TwInit(TW_OPENGL_CORE, NULL);
  TwWindowSize(windowWidth, windowHeight);
  CameraGUI = TwNewBar("CameraGUI");
  TwDefine(" GLOBAL help='Exercise Sheet 09 - Geometry shader.' "); // Message added to the help bar.
  
  // set position and size of AntTweakBar
  char *setbarposition = new char[1000];
  sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
  std::cout << setbarposition << std::endl;
  TwDefine(setbarposition);
  delete[] setbarposition;

  glutCloseFunc(close);
  
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << "(glewInit) - Error: " << glewGetErrorString(err) << std::endl;
  }
  std::cout << "(glewInit) - Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  
  // init stuff //
  initGL();

  // configure Camera
  camera.setFar(20.0);
  camera.setNear(0.01);
  camera.setOpeningAngle(45.0);
  camera.setAspect(1.2);
  
  // init object loader
  objLoader = new ObjLoader();

  // init matrix stacks with identity //
  glm_ProjectionMatrix.push(glm::mat4(1));
  glm_ModelViewMatrix.push(glm::mat4(1));
  
  initShader();
  initScene();

  // configure GUI editing bars
  updateBar();

  
  // start render loop //
  glutMainLoop();
  disableShader();

  // clean up allocated data //
  deleteShader(simpleShaderProg);
  // TODO : delete hair programs
  deleteShader(hairTrisProg);
  deleteShader(hairLineProg);

  close();
  
  
  return 0;
}

void updateBar() {

	TwRemoveAllVars(CameraGUI);

    std::stringstream label;
	
    label << " group='Material " << materialIndex << "' ";
	TwAddVarRW(CameraGUI, "Ambient M", TW_TYPE_COLOR3F, &currentMaterial->ambient_color, label.str().c_str());
	TwAddVarRW(CameraGUI, "Diffuse M", TW_TYPE_COLOR3F, &currentMaterial->diffuse_color, label.str().c_str());
	TwAddVarRW(CameraGUI, "Specular M", TW_TYPE_COLOR3F, &currentMaterial->specular_color, label.str().c_str());
	TwAddVarRW(CameraGUI, "Shininess M", TW_TYPE_FLOAT, &currentMaterial->specular_shininess, label.str().c_str());
		
	std::stringstream label2;
	label2.clear();
    label2 << " group='Hair'";

	// TODO:
	//	- add manual parameters for hair: length, width, segments, gravity
	//  - set the stepping and min/max values for each parameter to appropriate values
	TwAddVarRW(CameraGUI, "Hair length", TW_TYPE_FLOAT, &hairLength, "step=0.01");
	TwAddVarRW(CameraGUI, "Gravity", TW_TYPE_FLOAT, &gravity, "step=0.01");
	TwAddVarRW(CameraGUI, "Hair segments", ETwType::TW_TYPE_UINT32, &hairSegments, "step=1");
	TwAddVarRW(CameraGUI, "Hair witdh", TW_TYPE_FLOAT, &hairWidth, "step=0.01");

	label2.clear();
    label2 << " group='Object control " << lightIndex << "' ";
	TwAddVarRW(CameraGUI, "Render object", TW_TYPE_BOOL8 , &renderModel, "");
	TwAddVarRW(CameraGUI, "Render hair", TW_TYPE_BOOL8, &renderHair, "");
	TwAddVarRW(CameraGUI, "Render hair as lines", TW_TYPE_BOOL8, &renderHairLines, "");
}

// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
    // Set OpenGL viewport and camera
	windowWidth = width;
	windowHeight = height;

	camera.setAspect((float)width / (float)height);

    // Send the new window size to AntTweakBar
    TwWindowSize(windowWidth, windowHeight);

	// set position and size of AntTweakBar
	char *setbarposition = new char[1000];
	sprintf(setbarposition, " CameraGUI position='%i %i' size='%i %i'\0", windowWidth-(guiWidth-10), 20, guiWidth-20, windowHeight-40);
	TwDefine(setbarposition);
	delete[] setbarposition;

}

void close() {

	TwTerminate();
	// free allocated objects
	delete objLoader;
	std::cout << "Shutdown program." << std::endl;
}

void initShader() {


  // create simple illumination program
  simpleShaderProg = glCreateProgram();
  createShader(simpleShaderProg, "../../shader/simple.vert", "../../shader/simple.frag");
   
  // set address of fragment color output //
  glBindFragDataLocation(simpleShaderProg, 0, "color");
  
  // get uniform locations for common variables //
  uniformLocations["simple.projection"] = glGetUniformLocation(simpleShaderProg, "projection");
  uniformLocations["simple.modelview"] = glGetUniformLocation(simpleShaderProg, "modelview");
  uniformLocations["simple.view"] = glGetUniformLocation(simpleShaderProg, "view");

  // insert the uniform locations for all light and material properties
  uniformLocations["light.position"] = glGetUniformLocation(simpleShaderProg, "lightSource.position");
  uniformLocations["light.ambient"] = glGetUniformLocation(simpleShaderProg, "lightSource.ambient_color");
  uniformLocations["light.diffuse"] = glGetUniformLocation(simpleShaderProg, "lightSource.diffuse_color");
  uniformLocations["light.specular"] = glGetUniformLocation(simpleShaderProg, "lightSource.specular_color");
  // material unform locations //
  uniformLocations["material.ambient"] = glGetUniformLocation(simpleShaderProg, "material.ambient_color");
  uniformLocations["material.diffuse"] = glGetUniformLocation(simpleShaderProg, "material.diffuse_color");
  uniformLocations["material.specular"] = glGetUniformLocation(simpleShaderProg, "material.specular_color");
  uniformLocations["material.shininess"] = glGetUniformLocation(simpleShaderProg, "material.specular_shininess");

  // TODO: create hair triangles program and get uniforms
  createShader(hairTrisProg, "../../shader/hair.vert", "../../shader/hair.geom", "../../shader/hair.frag");
  uniformLocations["tris.hairLength"] = glGetUniformLocation(hairTrisProg, "hairLength");
  uniformLocations["tris.hairWidth"] = glGetUniformLocation(hairTrisProg, "hairWidth");
  uniformLocations["tris.hairSegments"] = glGetUniformLocation(hairTrisProg, "hairSegments");
  uniformLocations["tris.gravity"] = glGetUniformLocation(hairTrisProg, "gravity");

  uniformLocations["tris.projection"] = glGetUniformLocation(hairTrisProg, "projection");
  uniformLocations["tris.modelview"] = glGetUniformLocation(hairTrisProg, "modelview");
  uniformLocations["tris.view"] = glGetUniformLocation(hairTrisProg, "view");

  uniformLocations["materialt.ambient"] = glGetUniformLocation(hairTrisProg, "material.ambient_color");
  uniformLocations["materialt.diffuse"] = glGetUniformLocation(hairTrisProg, "material.diffuse_color");
  uniformLocations["materialt.specular"] = glGetUniformLocation(hairTrisProg, "material.specular_color");
  uniformLocations["materialt.shininess"] = glGetUniformLocation(hairTrisProg, "material.specular_shininess");

  // TODO: create hair lines program and get uniforms
  createShader(hairLineProg, "../../shader/hair.vert", "../../shader/hair_lines.geom", "../../shader/hair.frag");

  uniformLocations["line.hairLength"] = glGetUniformLocation(hairLineProg, "hairLength");
  uniformLocations["line.hairWidth"] = glGetUniformLocation(hairLineProg, "hairWidth");
  uniformLocations["line.hairSegments"] = glGetUniformLocation(hairLineProg, "hairSegments");
  uniformLocations["line.gravity"] = glGetUniformLocation(hairLineProg, "gravity");

  uniformLocations["line.projection"] = glGetUniformLocation(hairLineProg, "projection");
  uniformLocations["line.modelview"] = glGetUniformLocation(hairLineProg, "modelview");
  uniformLocations["line.view"] = glGetUniformLocation(hairLineProg, "view");

  uniformLocations["materialh.ambient"] = glGetUniformLocation(hairLineProg, "material.ambient_color");
  uniformLocations["materialh.diffuse"] = glGetUniformLocation(hairLineProg, "material.diffuse_color");
  uniformLocations["materialh.specular"] = glGetUniformLocation(hairLineProg, "material.specular_color");
  uniformLocations["materialh.shininess"] = glGetUniformLocation(hairLineProg, "material.specular_shininess");

  glBindFragDataLocation(hairLineProg, 0, "fragColor");
  glBindFragDataLocation(hairTrisProg, 0, "fragColor");

}

void initScene() {
  // load bunny.obj from disk and create renderable MeshObj //
  objLoader->loadObjFile("../../meshes/bunny.obj", "bunny");
  
  // init materials //
  Material mat;
  mat.ambient_color = glm::vec3(0.75, 0.375, 0.075);
  mat.diffuse_color = glm::vec3(0.75, 0.375, 0.075);
  mat.specular_color = glm::vec3(0.2, 0.2, 0.2);
  mat.specular_shininess = 5.0;
  materials.push_back(mat);
  
  mat.ambient_color = glm::vec3(0.0, 1.0, 0.0);
  mat.diffuse_color = glm::vec3(0.0, 1.0, 0.0);
  mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_shininess = 50.0;
  materials.push_back(mat);
  
  mat.ambient_color = glm::vec3(0.0, 0.0, 1.0);
  mat.diffuse_color = glm::vec3(0.0, 0.0, 1.0);
  mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_shininess = 150.0;
  materials.push_back(mat);
  
  mat.ambient_color = glm::vec3(1.0, 0.8, 0.3);
  mat.diffuse_color = glm::vec3(1.0, 0.8, 0.3);
  mat.specular_color = glm::vec3(1.0, 1.0, 1.0);
  mat.specular_shininess = 5.0;
  materials.push_back(mat);
  
  // save material count for later and select first material //
  materialCount = materials.size();
  materialIndex = 0;
  currentMaterial = &materials[materialIndex];
  
  // init lights //
  LightSource light;
  light.ambient_color = glm::vec3(0.1, 0.1, 0.1);
  light.diffuse_color = glm::vec3(1.0, 1.0, 1.0);
  light.specular_color = glm::vec3(1.0, 1.0, 1.0);
  light.position = glm::vec3(4, -4, 4);
  lights.push_back(light);
  
  light.ambient_color = glm::vec3(0.1, 0.1, 0.3);
  light.diffuse_color = glm::vec3(0.0, 0.5, 1.0);
  light.specular_color = glm::vec3(0.0, 0.5, 1.0);
  light.position = glm::vec3(0, 2, 0);
  lights.push_back(light);
  
  // save light source count for later and select first light source //
  lightCount = lights.size();
  lightIndex = 0;

  currentLight = &lights[lightIndex];
}

void renderScene() {

  // get projection mat from camera controller //
  glm_ProjectionMatrix.top() = camera.getProjectionMat();
  
  // get modelview mat from camera controller //
  glm_ModelViewMatrix.top() = camera.getModelViewMat();
  glm_ModelViewMatrix.push(glm_ModelViewMatrix.top());
  glm_ModelViewMatrix.top() *= glm::scale(glm::vec3(0.2));
    
  if (renderModel == true) {

	// activate simple illumination
	glUseProgram(simpleShaderProg);

	// upload projection matrix //
	glUniformMatrix4fv(uniformLocations["simple.projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));

	glUniformMatrix4fv(uniformLocations["simple.modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
	glUniformMatrix4fv(uniformLocations["simple.view"], 1, false, glm::value_ptr(camera.getModelViewMat()));

	// upload the properties of the currently chosen light source here //

	glUniform3fv(uniformLocations["light.position"], 1, glm::value_ptr(lights[lightIndex].position));
	glUniform3fv(uniformLocations["light.ambient"], 1, glm::value_ptr(lights[lightIndex].ambient_color));
	glUniform3fv(uniformLocations["light.diffuse"], 1, glm::value_ptr(lights[lightIndex].diffuse_color));
	glUniform3fv(uniformLocations["light.specular"], 1, glm::value_ptr(lights[lightIndex].specular_color));
  
	// upload the chosen material properties here //
	glUniform3fv(uniformLocations["material.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
	glUniform3fv(uniformLocations["material.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
	glUniform3fv(uniformLocations["material.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
	glUniform1f(uniformLocations["material.shininess"], materials[materialIndex].specular_shininess);

	// render the actual object //
	objLoader->getMeshObj("bunny")->render();
  }
  
  // TODO : render hair for scene object using the first hair rendering program (lines)
  //	- activate hair lines program
  //	- upload matrices for projection, modelview and view as usual
  //	- upload uniforms for hair parameters (length, gravity, material)
  //	- render the scene object

  // TODO : render hair for scene object using the first hair rendering program (triangles)
  //	- activate hair lines program
  //	- upload matrices for projection, modelview and view as usual
  //	- upload uniforms for hair parameters (length, width, segments, gravity, material)
  //	- render the scene object
  if (renderHair == true) {
	  
	  if (renderHairLines == true) {

		  // insert code here
		  glUseProgram(hairLineProg);
		  glUniformMatrix4fv(uniformLocations["line.projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));

		  glUniformMatrix4fv(uniformLocations["line.modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
		  glUniformMatrix4fv(uniformLocations["line.view"], 1, false, glm::value_ptr(camera.getModelViewMat()));

		  glUniform1f(uniformLocations["line.hairLength"], hairLength);
		  glUniform1f(uniformLocations["line.hairWidth"], hairWidth);
		  glUniform1i(uniformLocations["line.hairSegments"], hairSegments);
		  glUniform1f(uniformLocations["line.gravity"], gravity);

		  glUniform3fv(uniformLocations["materialh.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
		  glUniform3fv(uniformLocations["materialh.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
		  glUniform3fv(uniformLocations["materialh.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
		  glUniform1f(uniformLocations["materialh.shininess"], materials[materialIndex].specular_shininess);
		  
	  }
	  else {

		  // insert code her
		  glUseProgram(hairTrisProg);
		  glUniformMatrix4fv(uniformLocations["tris.projection"], 1, false, glm::value_ptr(glm_ProjectionMatrix.top()));

		  glUniformMatrix4fv(uniformLocations["tris.modelview"], 1, false, glm::value_ptr(glm_ModelViewMatrix.top()));
		  glUniformMatrix4fv(uniformLocations["tris.view"], 1, false, glm::value_ptr(camera.getModelViewMat()));

		  glUniform1f(uniformLocations["tris.hairLength"], hairLength);
		  glUniform1f(uniformLocations["tris.hairWidth"], hairWidth);
		  glUniform1i(uniformLocations["tris.hairSegments"], hairSegments);
		  glUniform1f(uniformLocations["tris.gravity"], gravity);

		  glUniform3fv(uniformLocations["materialt.ambient"], 1, glm::value_ptr(materials[materialIndex].ambient_color));
		  glUniform3fv(uniformLocations["materialt.diffuse"], 1, glm::value_ptr(materials[materialIndex].diffuse_color));
		  glUniform3fv(uniformLocations["materialt.specular"], 1, glm::value_ptr(materials[materialIndex].specular_color));
		  glUniform1f(uniformLocations["materialt.shininess"], materials[materialIndex].specular_shininess);
	 }

	  // insert code here
	  objLoader->getMeshObj("bunny")->render();

  }

  // restore scene graph to previous state //
  glm_ModelViewMatrix.pop();
  
}

void updateGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  // update camera
  camera.updateCamera();

  // set viewport dimensions //
  glViewport(0, 0, windowWidth, windowHeight);

  // render scene //
  renderScene();
  
  // Draw GUI //
  TwDraw();
	    
  // swap renderbuffers for smooth rendering //
  glutSwapBuffers();

}

void idle() {
  glutPostRedisplay();
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
	if (TwEventKeyboardGLUT(key, x, y) != 1) { // if GUI has not responded to event

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
		  currentMaterial = &materials[materialIndex];
		  updateBar();
		  break;
		}
	  }
	  glutPostRedisplay();
	}
}

void mouseEvent(int button, int state, int x, int y) {

  // use event in GUI
  if (TwEventMouseButtonGLUT(button, state, x, y) != 1) { // if GUI has not responded to event

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
}

void mouseMoveEvent(int x, int y) {

  // use event in GUI
  if (TwEventMouseMotionGLUT(x, y) != 1) { // if GUI has not responded to event

	  camera.updateMousePos(x, y);
	  glutPostRedisplay();
  }
}