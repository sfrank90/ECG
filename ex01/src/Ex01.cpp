#include "Ex01.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

void triangleSolution(void) {
  // TODO: implement your solution here //
	
	//definition of vectors
	glm::vec3 vec_a = glm::vec3(4.0, 1.0, 0.0);
	glm::vec3 vec_b = glm::vec3(-1.0, 3.0, 2.0);
	glm::vec3 vec_c = glm::vec3(0.0, 2.0, 1.0);

	glm::vec3 vec_v = glm::vec3(0.0, 2.0, 1.0);

	//calc of vectors
	glm::vec3 vec_ab = vec_b - vec_a;
	glm::vec3 vec_ac = vec_c - vec_a;

	//normal of triangle
	glm::vec3 triangle_n = glm::cross(vec_ab, vec_ac);

	//normalize
	glm::vec3 tn_n = glm::normalize(triangle_n);
	glm::vec3 vec_vn = glm::normalize(vec_v);

	// angle between normal and given vector
	float angle = glm::acos(glm::dot(tn_n, vec_vn));

	// conversion into degress
	angle = angle * 180 / M_PI;
	printf("Angle: %f\n", angle);
  
}

int main (int argc, char **argv) {
  triangleSolution();
  std::cin.get();
  return 0;
}
