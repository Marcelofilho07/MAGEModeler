#pragma once
#include "MModel.h"


struct Block {
	Vertex position;   
	glm::vec3 dimensions; 
	glm::vec3 orientation;
};

struct Sphere {
	Vertex position;   
	float radius;      
};

struct Cylinder {
	Vertex position;   
	float radius;      
	float height;      
	glm::vec3 orientation;
};

struct Cone {
	Vertex position;   
	float radius;      
	float height;      
	glm::vec3 orientation;
};