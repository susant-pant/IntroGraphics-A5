#include "camera.h"
#include <cstdio>

mat4 rotateAbout(vec3 axis, float radians)
{
	mat4 matrix;

	matrix[0][0] = cos(radians) + axis.x*axis.x*(1-cos(radians));
	matrix[1][0] = axis.x*axis.y*(1-cos(radians))-axis.z*sin(radians);
	matrix[2][0] = axis.x*axis.z*(1-cos(radians)) + axis.y*sin(radians);

	matrix[0][1] = axis.y*axis.x*(1-cos(radians)) + axis.z*sin(radians);
	matrix[1][1] = cos(radians) + axis.y*axis.y*(1-cos(radians));
	matrix[2][1] = axis.y*axis.z*(1-cos(radians)) - axis.x*sin(radians);

	matrix[0][2] = axis.z*axis.x*(1-cos(radians)) - axis.y*sin(radians);
	matrix[1][2] = axis.z*axis.y*(1-cos(radians)) + axis.x*sin(radians);
	matrix[2][2] = cos(radians) + axis.z*axis.z*(1-cos(radians));

	return matrix;
}

Camera::Camera():	dir(vec3(0, 0, -1)), 
					right(vec3(1, 0, 0)), 
					up(vec3(0, 1, 0)),
					pos(vec3(0,0,0)),
					sphereCoords(vec3(0,0,0)), //azimuth, altitude, radius
					atSphere(vec3(0,0,0)),
					sphereRadius(0.f)
{}

Camera::Camera(vec3 _sphereCoords, vec3 _atSphere, float _sphereRadius):
												sphereCoords(_sphereCoords),
												atSphere(_atSphere),
												sphereRadius(_sphereRadius)
{
	updateThings();	
}

void Camera::updateThings(){
	float theta = sphereCoords.y;
	float phi = sphereCoords.x;
	if (sphereCoords.z < ceil(sphereRadius)){
		sphereCoords.z = ceil(sphereRadius);
	}
	else if (sphereCoords.z > 75){
		sphereCoords.z = 75;
	}
	pos = sphereCoords.z * vec3(cos(theta) * sin(phi),
						sin(theta) * sin(phi),
						cos(phi)) + atSphere;
	dir = normalize(pos - atSphere);
	right = normalize(cross(dir, vec3(0, 0, 1)));
	up =  normalize(cross(right, dir));
}

/*
	[ Right 0 ]
	[ Up 	0 ]
	[ -Dir	0 ]
	[ 0 0 0 1 ]
*/

mat4 Camera::getMatrix()
{
	mat4 cameraRotation = mat4(
			vec4(right, 0),
			vec4(up, 0),
			vec4(-dir, 0),
			vec4(0, 0, 0, 1));

	mat4 translation = mat4 (
			vec4(1, 0, 0, 0),
			vec4(0, 1, 0, 0),
			vec4(0, 0, 1, 0),
			vec4(pos, 1));

	return transpose(cameraRotation)*translation;
}

void Camera::moveCamera(float deltaPhi, float deltaTheta, float deltaRadius)
{
	sphereCoords += vec3(deltaPhi, deltaTheta, deltaRadius);
	updateThings();
}