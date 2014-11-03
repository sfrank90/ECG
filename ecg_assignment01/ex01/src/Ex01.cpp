#include "Ex01.h"
#include <math.h>

#include "../include/glm/glm.hpp"

void triangleSolution(void) {
  // TODO: implement your solution here //
    glm::vec3 vec_a = glm::vec3(4.0, 1.0, 0.0);
    glm::vec3 vec_b = glm::vec3(-1.0, 3.0, 2.0);
    glm::vec3 vec_c = glm::vec3(0.0, 2.0, 1.0);

    glm::vec3 vec_v = glm::vec3(0.0, 2.0, 1.0);

    glm::vec3 vec_ab = vec_b - vec_a;
    glm::vec3 vec_ac = vec_c - vec_a;

    glm::vec3 triangle_n = glm::cross(vec_ab, vec_ac);

    glm::vec3 tn_n = glm::normalize(triangle_n);
    glm::vec3 vec_vn = glm::normalize(vec_v);
    float angle = glm::acos(glm::dot(tn_n, vec_vn));
    angle = angle * 180/M_PI;
    printf("Angle: %f\n", angle);
  
}

int main (int argc, char **argv) {
  triangleSolution();
  
  return 0;
}
