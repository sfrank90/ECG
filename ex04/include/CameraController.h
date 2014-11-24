#ifndef __CAMERA_CONTROLLER__
#define __CAMERA_CONTROLLER__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STEP_DISTANCE 0.05f

class CameraController {
  public:
    CameraController(float theta = 0, float phi = 0, float dist = 1);
    ~CameraController();
    
    enum MouseState {NO_BTN = 0, LEFT_BTN, RIGHT_BTN, MIDDLE_BTN};
    enum Motion {MOVE_FORWARD = 0, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN};
    
    // update mouse input //
    void updateMousePos(int x, int y);
    void updateMouseBtn(MouseState state, int x, int y);
    
    // move camera in one of the four defined directions //
    void move(Motion motion);
    
    // reset camera parameters //
    void reset(float theta = 0, float phi = 0, float dist = 1);
    void resetOrientation(float theta = 0.0f, float phi = 0.0f, float dist = 1.0f);
    void resetIntrinsics(float angle = 45.0f, float aspect = 1.0f, float near = 0.1f, float far = 50.0f);
    
    // get/set camera parameters //
    void setNear(float near);
    float getNear(void);
    void setFar(float near);
    float getFar(void);
    void setOpeningAngle(float angle);
    float getOpeningAngle(void);
    void setAspect(float ratio);
    float getAspect(void);
    
    // get camera configuration //
    glm::mat4 getProjectionMat(void);
    glm::mat4 getModelViewMat(void);
    glm::vec3 getCameraPosition(void);
    
  public:
    float mNear, mFar, mOpenAngle, mAspect;
	float mTheta, mPhi;
	glm::vec3 mCameraPosition;

  private:
    float mLastTheta, mLastPhi;
    
    int mX, mY;
    MouseState mState;
};

#endif
