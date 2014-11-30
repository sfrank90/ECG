#include "CameraController.h"
#include <iostream>
#include <cmath>

CameraController::CameraController(float theta, float phi, float dist) {
  reset(theta, phi, dist);
}

CameraController::~CameraController() {}
    
void CameraController::updateMousePos(int x, int y) {
	float pitchSensivity = 2.0f, yawSensivity = 2.0f;
  switch (mState) {
    case LEFT_BTN : {
      // TODO: left button pressed -> compute position difference to click-point and compute new angles //
		float distX = mX - x;
		float distY = mY - y;
		std::cout << "Mouse Moving: " << distX << " : " << distY << std::endl;
		mTheta = mLastTheta - (float)(mX - x) / yawSensivity;
		mPhi = mLastPhi - (float)(mY - y) / pitchSensivity;
      break;
    }
    case RIGHT_BTN : {
      // not used yet //
      break;
    }
    default : break;
  }
}

void CameraController::updateMouseBtn(MouseState state, int x, int y) {
  switch (state) {
    case NO_BTN : {
      // TODO: button release -> save current angles for later rotations //
		mLastPhi = mPhi;
		mLastTheta = mTheta;
      break;
    }
    case LEFT_BTN : {
      // TODO: left button has been pressed -> start new rotation -> save initial point //
		mX = x;
		mY = y;
      break;
    }
    case RIGHT_BTN : {
      // not used yet //
      break;
    }
    default : break;
  }
  mState = state;
}

void CameraController::move(Motion motion) {
  // init direction multiplicator (forward/backward, left/right are SYMMETRIC!) //
  int dir = 1;
  std::cout << "(Camera Controller) Position movement: " << motion;
  switch (motion) {
    // TODO: move camera along or perpendicular to its viewing direction or along y according to motion state //
    //       motion state is one of: (MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN)
	  case MOVE_FORWARD:
		  dir = -1;
		  std::cout << " - forward";
	  case MOVE_BACKWARD:
		  mCameraPosition[2] += dir * STEP_DISTANCE;
		  if (dir == 1)
			std::cout << " - backward";
		  break;
	  case MOVE_LEFT:
		  dir = -1;
		  std::cout << " - left";
	  case MOVE_RIGHT:
		  mCameraPosition[0] += dir * STEP_DISTANCE;
		  if (dir == 1)
			std::cout << " - right";
		  break;
	  case MOVE_UP:
		  dir = -1;
		  std::cout << " - up";
	  case MOVE_DOWN:
		  mCameraPosition[1] -= dir * STEP_DISTANCE;
		  if (dir == 1)
			std::cout << " - down";
		  break;

	default : 
		// Error Handling
		std::cout << " undefined move-Action" << std::endl;
		exit(-1);
		break;
  }
  std::cout << std::endl;
}

glm::mat4 CameraController::getProjectionMat(void) {
  // TODO:  Return perspective matrix describing the camera intrinsics.
  //		The perspective matrix has been derived in the lecture.
	glm::mat4 projectionMat = glm::perspective(mOpenAngle, mAspect, mNear, mFar);
  return projectionMat;
}

glm::mat4 CameraController::getModelViewMat(void) {
  // TODO: return the modelview matrix describing the position and orientation of the camera //
  //       compute a simple lookAt position relative to the camera's position                //
  glm::mat4 modelViewMat;
  glm::vec3 center(0.0, 0.0, 1.0);
  center = mCameraPosition - center;

  glm::vec3 up(0.0, 1.0, 0.0);

  modelViewMat = glm::lookAt(mCameraPosition, center, up);
  modelViewMat = glm::rotate(modelViewMat, mPhi, glm::vec3(1.0, 0.0, 0.0));
  modelViewMat = glm::rotate(modelViewMat, mTheta, glm::vec3(0.0, 1.0, 0.0));

  return modelViewMat;
}

void CameraController::reset(float theta, float phi, float dist) {
  // reset everything //
  resetOrientation(theta, phi, dist);
  resetIntrinsics();
  mX = 0;
  mY = 0;
  mState = NO_BTN;
}

void CameraController::resetOrientation(float theta, float phi, float dist) {
  // reset camera extrisics //
  mPhi = phi;
  mLastPhi = mPhi;
  mTheta = theta;
  mLastTheta = mTheta;
  // move camera about 'dist' along opposite of rotated view vector //
  mCameraPosition = glm::vec3(0 ,
			      0,
			       dist);
  // lookAt position is now (0, 0, 0) //
}

void CameraController::resetIntrinsics(float angle, float aspect, float near, float far) {
  // reset intrinsic parameters //
  setOpeningAngle(angle);
  setAspect(aspect);
  setNear(near);
  setFar(far);
}

void CameraController::setNear(float near) {
  mNear = near;
}

float CameraController::getNear(void) {
  return mNear;
}

void CameraController::setFar(float far) {
  mFar = far;
}

float CameraController::getFar(void) {
  return mFar;
}

void CameraController::setOpeningAngle(float angle) {
  mOpenAngle = angle;
}

float CameraController::getOpeningAngle(void) {
  return mOpenAngle;
}

void CameraController::setAspect(float ratio) {
  mAspect = ratio;
}

float CameraController::getAspect(void) {
  return mAspect;
}

glm::vec3 CameraController::getCameraPosition(void) {
  return mCameraPosition;
}
