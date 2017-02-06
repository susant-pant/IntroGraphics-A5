#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"

using namespace glm;

class Camera{
public:
	vec3 dir;
	vec3 up;
	vec3 right;
	vec3 pos;
	vec3 sphereCoords;
	vec3 atSphere;
	float sphereRadius;

	Camera();
	Camera(vec3 _sphereCoords, vec3 _atSphere, float _sphereRadius);

	void updateThings();
	mat4 getMatrix();

	void moveCamera(float deltaPhi, float deltaTheta, float deltaRadius);

};

#endif