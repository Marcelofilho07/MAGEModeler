#pragma once
#include <cstdint>
#include <array>
#include <algorithm>
#include "MModel.h"
#include "Primitives.h"
/*
 * WHITE = NO COLLISION;
 * BLACK = LEAF NODE;
 * GREY = COLLISION;
 */
enum NodeCode {WHITE, BLACK, GREY}; 
enum Operation {INTERSECTION, UNION, DIFFERENCE}; 

struct OctreeNode
{
	Vertex posMin;
	Vertex posMax;
	
	std::array<OctreeNode*, 8> children{};
	
	NodeCode code = WHITE;

};

struct Octree
{
	OctreeNode* root;
};

void populateFromOctree(OctreeNode* node, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint32_t& currentIndex);

OctreeNode createNodeForSphere(const Sphere& sphere);
OctreeNode createNodeForBlock(const Block& block);
OctreeNode createNodeForCylinder(const Cylinder& cylinder);
OctreeNode createNodeForCone(const Cone& cone);
void buildTree(MModel& model, OctreeNode& oct, int depth, std::string& code);
void buildTree(Sphere& model, OctreeNode& oct, int depth, std::string& code);
void buildTree(Block& model, OctreeNode& oct, int depth, std::string& code);
void buildTree(Cylinder& model, OctreeNode& oct, int depth, std::string& code);
void buildTree(Cone& model, OctreeNode& oct, int depth, std::string& code);

void readCodeAndPopulateTree(OctreeNode& oct, std::string& code);

OctreeNode buildTreeFromCode(std::string& code, glm::vec3 posMin = glm::vec3(-5.0f, -5.0f, -5.0f), glm::vec3 posMax = glm::vec3(5.0f, 5.0f, 5.0f));

OctreeNode buildInitialBoundingBox(MModel& m);

void buildTreeFromBooleanOperation(const OctreeNode& rootA, const OctreeNode& rootB, OctreeNode& newTree, const Operation& operation, std::string& code, bool bAisNull = false, bool bBisNull = false);

float getOctreeVolume(OctreeNode* node);

void getOctreeBlackNodes(OctreeNode* node, std::vector<OctreeNode*>& blackNodes);

float getOctreeArea(OctreeNode& node);

bool isCollidingAABB_Sphere(const OctreeNode& cube, const Sphere& sphere);
bool isCollidingAABB_Block(const OctreeNode& cube, const Block& block);
bool isCollidingAABB_Cylinder(const OctreeNode& cube, const Cylinder& cylinder);
bool isCollidingAABB_Cone(const OctreeNode& cube, const Cone& cone);
