#include "CameraController.h"

CameraController::CameraController(float theta, float phi, float dist) {
  reset(theta, phi, dist);
}

CameraController::~CameraController() {}
    
void CameraController::updateMousePos(int x, int y) {
  switch (mState) {
    case LEFT_BTN : {
      // TODO: left button pressed -> compute position difference to click-point and compute new angles //
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
      break;
    }
    case LEFT_BTN : {
      // TODO: left button has been pressed -> start new rotation -> save initial point //
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
  switch (motion) {
    // TODO: move camera along or perpendicular to its viewing direction or along y according to motion state //
    //       motion state is one of: (MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN)
    default : break;
  }
}

glm::mat4 CameraController::getProjectionMat(void) {
  // TODO:  Return perspective matrix describing the camera intrinsics.
  //		The perspective matrix has been derived in the lecture.
  glm::mat4 projectionMat;
  return projectionMat;
}

glm::mat4 CameraController::getModelViewMat(void) {
  // TODO: return the modelview matrix describing the position and orientation of the camera //
  //       compute a simple lookAt position relative to the camera's position                //
  glm::mat4 modelViewMat;
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
  mCameraPosition = glm::vec3(sin(mTheta) * cos(mPhi) * dist,
			      sin(mPhi) * dist,
			      cos(mTheta) * cos(mPhi) * dist);
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
