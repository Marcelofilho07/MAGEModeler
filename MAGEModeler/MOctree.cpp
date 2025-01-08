#include "MOctree.h"

void projectVertices(const std::vector<glm::vec3>& vertices, const glm::vec3& axis, float& min, float& max) {
	min = max = glm::dot(vertices[0], axis);
	for (size_t i = 1; i < vertices.size(); ++i) {
		float projection = glm::dot(vertices[i], axis);
		min = std::min(min, projection);
		max = std::max(max, projection);
	}
}

void projectVertices(const std::array<glm::vec3, 8>& vertices, const glm::vec3& axis, float& min, float& max) {
	min = max = glm::dot(vertices[0], axis);
	for (size_t i = 1; i < vertices.size(); ++i) {
		float projection = glm::dot(vertices[i], axis);
		min = std::min(min, projection);
		max = std::max(max, projection);
	}
}

bool checkCollisionSAT(const std::array<glm::vec3, 8>& cubeVertices, const std::vector<glm::vec3>& faceVertices) {
    // Eixos do cubo
    std::array<glm::vec3, 3> cubeAxes = {
        glm::vec3(1, 0, 0),
        glm::vec3(0, 1, 0),
        glm::vec3(0, 0, 1)
    };

    // Eixo normal da face
    glm::vec3 faceNormal = glm::normalize(glm::cross(faceVertices[1] - faceVertices[0], faceVertices[2] - faceVertices[0]));

    // Eixos derivados das arestas
    std::vector<glm::vec3> edgeAxes;
    for (size_t i = 0; i < faceVertices.size(); ++i) {
        glm::vec3 edge = faceVertices[(i + 1) % faceVertices.size()] - faceVertices[i];
        for (const auto& cubeAxis : cubeAxes) {
            edgeAxes.push_back(glm::normalize(glm::cross(edge, cubeAxis)));
        }
    }

    // Combinar todos os eixos
    std::vector<glm::vec3> axes;
    axes.insert(axes.end(), cubeAxes.begin(), cubeAxes.end());
    axes.push_back(faceNormal);
    axes.insert(axes.end(), edgeAxes.begin(), edgeAxes.end());

    // Testar separação para cada eixo
    for (const auto& axis : axes) {
        if (axis == glm::vec3(0, 0, 0)) {
            continue;
        }

        float minCube, maxCube;
        projectVertices(cubeVertices, axis, minCube, maxCube);

        float minFace, maxFace;
        projectVertices(faceVertices, axis, minFace, maxFace);

        if (maxCube < minFace || maxFace < minCube) {
            return false; // Separação encontrada
        }
    }

    return true; // Nenhuma separação encontrada, colisão ocorre
}



NodeCode classify(MModel& model, OctreeNode& oct) {
	glm::vec3 cubeMin = oct.posMin.pos;
	glm::vec3 cubeMax = oct.posMax.pos;

	// Calcular os 8 vértices do cubo
	std::array<glm::vec3, 8> cubeVertices = {
		glm::vec3(cubeMin.x, cubeMin.y, cubeMin.z),
		glm::vec3(cubeMax.x, cubeMin.y, cubeMin.z),
		glm::vec3(cubeMin.x, cubeMax.y, cubeMin.z),
		glm::vec3(cubeMax.x, cubeMax.y, cubeMin.z),
		glm::vec3(cubeMin.x, cubeMin.y, cubeMax.z),
		glm::vec3(cubeMax.x, cubeMin.y, cubeMax.z),
		glm::vec3(cubeMin.x, cubeMax.y, cubeMax.z),
		glm::vec3(cubeMax.x, cubeMax.y, cubeMax.z)
	};

	for (const Face& face : model.facesList) {
		std::vector<glm::vec3> faceVertices;
		for (const Vertex& vertex : face.vertexList) {
			faceVertices.push_back(vertex.pos);
		}

		if (faceVertices.size() < 3) {
			continue;
		}

		if (checkCollisionSAT(cubeVertices, faceVertices)) {
			return GREY;
		}
	}

	return WHITE;
}

bool isCollidingAABB_Sphere(const OctreeNode& cube, const Sphere& sphere)
{
	float closestX = std::max(cube.posMin.pos.x, std::min(sphere.position.pos.x, cube.posMax.pos.x));
	float closestY = std::max(cube.posMin.pos.y, std::min(sphere.position.pos.y, cube.posMax.pos.y));
	float closestZ = std::max(cube.posMin.pos.z, std::min(sphere.position.pos.z, cube.posMax.pos.z));
    
	float distanceSq = (closestX - sphere.position.pos.x) * (closestX - sphere.position.pos.x) +
					   (closestY - sphere.position.pos.y) * (closestY - sphere.position.pos.y) +
					   (closestZ - sphere.position.pos.z) * (closestZ - sphere.position.pos.z);
                       
	return distanceSq <= (sphere.radius * sphere.radius);
}

bool isCollidingAABB_Block(const OctreeNode& cube, const Block& block)
{
	return (cube.posMin.pos.x <= block.position.pos.x + block.dimensions.x / 2 &&
			cube.posMax.pos.x >= block.position.pos.x - block.dimensions.x / 2 &&
			cube.posMin.pos.y <= block.position.pos.y + block.dimensions.y / 2 &&
			cube.posMax.pos.y >= block.position.pos.y - block.dimensions.y / 2 &&
			cube.posMin.pos.z <= block.position.pos.z + block.dimensions.z / 2 &&
			cube.posMax.pos.z >= block.position.pos.z - block.dimensions.z / 2);
}

bool isCollidingAABB_Cylinder(const OctreeNode& cube, const Cylinder& cylinder)
{
	float closestX = std::max(cube.posMin.pos.x, std::min(cylinder.position.pos.x, cube.posMax.pos.x));
	float closestY = std::max(cube.posMin.pos.y, std::min(cylinder.position.pos.y, cube.posMax.pos.y));

	float distanceSq = (closestX - cylinder.position.pos.x) * (closestX - cylinder.position.pos.x) +
					   (closestY - cylinder.position.pos.y) * (closestY - cylinder.position.pos.y);
    
	bool intersectsBase = distanceSq <= (cylinder.radius * cylinder.radius);
	bool intersectsHeight = (cube.posMin.pos.z <= cylinder.position.pos.z + cylinder.height &&
							 cube.posMax.pos.z >= cylinder.position.pos.z);
                             
	return intersectsBase && intersectsHeight;
}

bool isCollidingAABB_Cone(const OctreeNode& cube, const Cone& cone)
{
	float closestX = std::max(cube.posMin.pos.x, std::min(cone.position.pos.x, cube.posMax.pos.x));
	float closestY = std::max(cube.posMin.pos.y, std::min(cone.position.pos.y, cube.posMax.pos.y));
    
	float distanceSq = (closestX - cone.position.pos.x) * (closestX - cone.position.pos.x) +
					   (closestY - cone.position.pos.y) * (closestY - cone.position.pos.y);
                       
	float effectiveRadius = cone.radius * (1 - (cube.posMax.pos.z - cone.position.pos.z) / cone.height);
	bool intersectsBase = distanceSq <= (effectiveRadius * effectiveRadius);
	bool intersectsHeight = (cube.posMin.pos.z <= cone.position.pos.z + cone.height &&
							 cube.posMax.pos.z >= cone.position.pos.z);
                             
	return intersectsBase && intersectsHeight;
}

NodeCode classify(const Block& block, OctreeNode& oct) {
	if (isCollidingAABB_Block(oct, block)) {
		return GREY;
	}
	return WHITE;
}

NodeCode classify(const Cylinder& cylinder, OctreeNode& oct) {
	if (isCollidingAABB_Cylinder(oct, cylinder)) {
		return GREY;
	}
	return WHITE;
}

NodeCode classify(const Sphere& sphere, OctreeNode& oct) {
	if (isCollidingAABB_Sphere(oct, sphere)) {
		return GREY;
	}
	return WHITE;
}

NodeCode classify(const Cone& cone, OctreeNode& oct) {
	if (isCollidingAABB_Cone(oct, cone)) {
		return GREY;
	}
	return WHITE;
}

char codeToChar(NodeCode code)
{
	if(code == BLACK)
		return 'B';
	if(code == WHITE)
		return 'W';
	return 'G';
}

void populateFromOctree(OctreeNode* node, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, uint32_t& currentIndex)
{
	if (!node || node->code == WHITE) return;
    
	if(node->code == BLACK)
	{
		glm::vec3 cubeVertices[] = {
			{node->posMin.pos.x, node->posMin.pos.y, node->posMin.pos.z},
			{node->posMax.pos.x, node->posMin.pos.y, node->posMin.pos.z},
			{node->posMax.pos.x, node->posMax.pos.y, node->posMin.pos.z},
			{node->posMin.pos.x, node->posMax.pos.y, node->posMin.pos.z},
			{node->posMin.pos.x, node->posMin.pos.y, node->posMax.pos.z},
			{node->posMax.pos.x, node->posMin.pos.y, node->posMax.pos.z},
			{node->posMax.pos.x, node->posMax.pos.y, node->posMax.pos.z},
			{node->posMin.pos.x, node->posMax.pos.y, node->posMax.pos.z}
		};

		for (const auto& pos : cubeVertices) {
			glm::vec3 localPos = (pos - node->posMin.pos) / (node->posMax.pos - node->posMin.pos);
			glm::vec3 color = localPos;
			vertices.push_back({pos, color});
		}

		uint32_t cubeIndices[] = {
			0, 1, 2,  0, 2, 3,
			4, 5, 6,  4, 6, 7,
			0, 1, 5,  0, 5, 4,
			2, 3, 7,  2, 7, 6,
			0, 3, 7,  0, 7, 4,
			1, 2, 6,  1, 6, 5 
		};

		for (uint32_t i = 0; i < 36; ++i) {
			indices.push_back(currentIndex + cubeIndices[i]);
		}

		currentIndex += 8;
	}

	else
	{
		for (OctreeNode* child : node->children) {
			populateFromOctree(child, vertices, indices, currentIndex);
		}
	}
}


void subdivide(OctreeNode& oct)
{
	glm::vec3 halfSize = (oct.posMax.pos - oct.posMin.pos) / 2.0f;
	for (int i = 0; i < 8; ++i) {
		oct.children[i] = new OctreeNode();
		oct.children[i]->posMin.pos = oct.posMin.pos + glm::vec3(i % 2, (i / 2) % 2, i / 4) * halfSize;
		oct.children[i]->posMax.pos = oct.children[i]->posMin.pos + halfSize;
	}
}

void buildTree(MModel& model, OctreeNode& oct, int depth, std::string& code)
{
	oct.code = classify(model, oct); //returns white, grey;
	if(oct.code == GREY)
	{
		if(depth == 0)
		{
			oct.code = BLACK;
			code += 'B';
		}
		else
		{
			subdivide(oct);
			bool bBlackBranch = true;
			
			code += '(';
			for(OctreeNode* childrenNode : oct.children)
			{
				buildTree(model, *childrenNode, depth - 1, code);
				if(childrenNode->code != BLACK)
				{
					bBlackBranch = false;
				}
			}
			if(bBlackBranch)
			{
				for(OctreeNode* childrenNode : oct.children)
				{
					code.pop_back();
				}
				code.pop_back();
				oct.code = BLACK;
			}
			else
			{
				code += ')';
			}
		}
	}

	if(oct.code == WHITE)
	{
		code += 'W';
	}
}

void buildTree(Sphere& model, OctreeNode& oct, int depth, std::string& code)
{
	oct.code = classify(model, oct); //returns white, grey;
	if(oct.code == GREY)
	{
		if(depth == 0)
		{
			oct.code = BLACK;
			code += 'B';
		}
		else
		{
			subdivide(oct);
			bool bBlackBranch = true;
			
			code += '(';
			for(OctreeNode* childrenNode : oct.children)
			{
				buildTree(model, *childrenNode, depth - 1, code);
				if(childrenNode->code != BLACK)
				{
					bBlackBranch = false;
				}
			}
			if(bBlackBranch)
			{
				for(OctreeNode* childrenNode : oct.children)
				{
					code.pop_back();
				}
				code.pop_back();
				oct.code = BLACK;
			}
			else
			{
				code += ')';
			}
		}
	}

	if(oct.code == WHITE)
	{
		code += 'W';
	}
}

void buildTree(Block& model, OctreeNode& oct, int depth, std::string& code)
{
	oct.code = classify(model, oct); //returns white, grey;
	if(oct.code == GREY)
	{
		if(depth == 0)
		{
			oct.code = BLACK;
			code += 'B';
		}
		else
		{
			subdivide(oct);
			bool bBlackBranch = true;
			
			code += '(';
			for(OctreeNode* childrenNode : oct.children)
			{
				buildTree(model, *childrenNode, depth - 1, code);
				if(childrenNode->code != BLACK)
				{
					bBlackBranch = false;
				}
			}
			if(bBlackBranch)
			{
				for(OctreeNode* childrenNode : oct.children)
				{
					//code.pop_back();
				}
				//code.pop_back();
				oct.code = BLACK;
			}
			else
			{
				code += ')';
			}
		}
	}

	if(oct.code == WHITE)
	{
		code += 'W';
	}
}

void buildTree(Cylinder& model, OctreeNode& oct, int depth, std::string& code)
{
	oct.code = classify(model, oct); //returns white, grey;
	if(oct.code == GREY)
	{
		if(depth == 0)
		{
			oct.code = BLACK;
			code += 'B';
		}
		else
		{
			subdivide(oct);
			bool bBlackBranch = true;
			
			code += '(';
			for(OctreeNode* childrenNode : oct.children)
			{
				buildTree(model, *childrenNode, depth - 1, code);
				if(childrenNode->code != BLACK)
				{
					bBlackBranch = false;
				}
			}
			if(bBlackBranch)
			{
				for(OctreeNode* childrenNode : oct.children)
				{
					//code.pop_back();
				}
				//code.pop_back();
				oct.code = BLACK;
			}
			else
			{
				code += ')';
			}
		}
	}

	if(oct.code == WHITE)
	{
		code += 'W';
	}
}

void buildTree(Cone& model, OctreeNode& oct, int depth, std::string& code)
{
	oct.code = classify(model, oct); //returns white, grey;
	if(oct.code == GREY)
	{
		if(depth == 0)
		{
			oct.code = BLACK;
			code += 'B';
		}
		else
		{
			subdivide(oct);
			bool bBlackBranch = true;
			
			code += '(';
			for(OctreeNode* childrenNode : oct.children)
			{
				buildTree(model, *childrenNode, depth - 1, code);
				if(childrenNode->code != BLACK)
				{
					bBlackBranch = false;
				}
			}
			if(bBlackBranch)
			{
				for(OctreeNode* childrenNode : oct.children)
				{
					//code.pop_back();
				}
				//code.pop_back();
				oct.code = BLACK;
			}
			else
			{
				code += ')';
			}
		}
	}

	if(oct.code == WHITE)
	{
		code += 'W';
	}
}


void readCodeAndPopulateTree(OctreeNode& oct, std::string::iterator& code, std::string::iterator& end)
{
	if(code == end)
		return;

	++code;
	
	if(*code == 'B')
	{
		oct.code = BLACK;
	}

	else if(*code == 'W')
	{
		oct.code = WHITE;
	}
	
	else if(*code == '(')
	{
		oct.code = GREY;
		subdivide(oct);
		for(OctreeNode* childrenNode : oct.children)
		{
			readCodeAndPopulateTree(*childrenNode, code, end);
		}
		
		++code;
	}
	
}

OctreeNode buildTreeFromCode(std::string& code, glm::vec3 posMin, glm::vec3 posMax)
{
	OctreeNode root;
	root.posMin.pos = posMin;
	root.posMax.pos = posMax;
	std::string::iterator codeIterator = code.begin();
	std::string::iterator end = code.end();
	
	if(*codeIterator == 'B')
	{
		root.code = BLACK;
	}
	
	else if(*codeIterator == 'W')
	{
		root.code = WHITE;
	}
	
	else if(*codeIterator == '(')
	{
		root.code = GREY;
		subdivide(root);
		for(OctreeNode* childrenNode : root.children)
		{
			readCodeAndPopulateTree(*childrenNode, codeIterator, end);
		}
		
		++codeIterator;
	}

	return root;
}

OctreeNode createNodeForSphere(const Sphere& sphere) {
	OctreeNode node{};
	node.posMin.pos = {sphere.position.pos.x - sphere.radius, 
				   sphere.position.pos.y - sphere.radius, 
				   sphere.position.pos.z - sphere.radius};
	node.posMax.pos = {sphere.position.pos.x + sphere.radius, 
				   sphere.position.pos.y + sphere.radius, 
				   sphere.position.pos.z + sphere.radius};
	return node;
}

OctreeNode createNodeForBlock(const Block& block) {
	const float maxDimension = std::max({block.dimensions.x, block.dimensions.y, block.dimensions.z});
    
	const glm::vec3 halfCube = glm::vec3(maxDimension / 2.0f);

	OctreeNode node{};
	node.posMin.pos = {block.position.pos - halfCube};
	node.posMax.pos = {block.position.pos + halfCube};
	
	return node;
}

OctreeNode createNodeForCylinder(const Cylinder& cylinder) {
	OctreeNode node{};
	node.posMin.pos = {cylinder.position.pos.x - cylinder.radius, 
				   cylinder.position.pos.y - cylinder.radius, 
				   cylinder.position.pos.z};
	node.posMax.pos = {cylinder.position.pos.x + cylinder.radius, 
				   cylinder.position.pos.y + cylinder.radius, 
				   cylinder.position.pos.z + cylinder.height};
	return node;
}

OctreeNode createNodeForCone(const Cone& cone) {
	OctreeNode node{};
	node.posMin.pos = {cone.position.pos.x - cone.radius, 
				   cone.position.pos.y - cone.radius, 
				   cone.position.pos.z};
	node.posMax.pos = {cone.position.pos.x + cone.radius, 
				   cone.position.pos.y + cone.radius, 
				   cone.position.pos.z + cone.height};
	return node;
}

OctreeNode buildInitialBoundingBox(MModel& m)
{
	Vertex maxPos{};
	maxPos.pos.x = -9999.f;
	maxPos.pos.y = -9999.f;
	maxPos.pos.z = -9999.f;
	Vertex minPos{};
	minPos.pos.x = 9999.f;
	minPos.pos.y = 9999.f;
	minPos.pos.z = 9999.f;
	for(auto v : m.vertices)
	{
		//Get max pos
		if(v.pos.x > maxPos.pos.x)
		{
			maxPos.pos.x = v.pos.x;
		}
		if(v.pos.y > maxPos.pos.y)
		{
			maxPos.pos.y = v.pos.y;
		}
		if(v.pos.z > maxPos.pos.z)
		{
			maxPos.pos.z = v.pos.z;
		}

		//Get min pos
		if(v.pos.x < minPos.pos.x)
		{
			minPos.pos.x = v.pos.x;
		}
		if(v.pos.y < minPos.pos.y)
		{
			minPos.pos.y = v.pos.y;
		}
		if(v.pos.z < minPos.pos.z)
		{
			minPos.pos.z = v.pos.z;
		}
	}

	Vertex centerPos{};

	centerPos.pos.x = (minPos.pos.x + maxPos.pos.x) / 2;
	centerPos.pos.y = (minPos.pos.y + maxPos.pos.y) / 2;
	centerPos.pos.z = (minPos.pos.z + maxPos.pos.z) / 2;

	float sideX = maxPos.pos.x - minPos.pos.x;
	float sideY = maxPos.pos.y - minPos.pos.y;
	float sideZ = maxPos.pos.z - minPos.pos.z;

	float greatestSide = std::max(sideX,sideY);
	greatestSide = std::max(greatestSide, sideZ);
	float halfSide = greatestSide /2.f;

	Vertex finalPosMin{{centerPos.pos.x - halfSide, centerPos.pos.y - halfSide, centerPos.pos.z - halfSide}};
	Vertex finalPosMax{{centerPos.pos.x + halfSide, centerPos.pos.y + halfSide, centerPos.pos.z + halfSide}};

	return OctreeNode{finalPosMin,finalPosMax};
}

void boolOperationOneIsLeaf(const OctreeNode& rootA, const OctreeNode& rootB, OctreeNode& newTree, const Operation& operation, std::string& code)
{
	if (rootA.code == BLACK) {
		if (operation == INTERSECTION) {
			newTree.code = rootB.code;
			code += 'G';
		}
		else{
			newTree.code = BLACK;
			code += 'B';
		}
	}
	else {
		if (operation == INTERSECTION) {
			newTree.code =  WHITE;
			code += 'W';
		}
		else{
			newTree = rootB;
			code += 'G';
		}
	}
}

float getOctreeVolume(OctreeNode* node)
{
	float volume = 0.f;
	if (!node || node->code == WHITE) return volume;
    
	if(node->code == BLACK)
	{
		glm::vec3 dimensoes = node->posMax.pos - node->posMin.pos;

		if (dimensoes.x < 0 || dimensoes.y < 0 || dimensoes.z < 0) {
			return -99999.0f;
		}
		
		return dimensoes.x * dimensoes.y * dimensoes.z;
	}
	else
	{
		for (OctreeNode* child : node->children) {
			volume += getOctreeVolume(child);
		}
	}

	return volume;
}

void getOctreeBlackNodes(OctreeNode* node, std::vector<OctreeNode*>& blackNodes)
{
	if (!node || node->code == WHITE) return;
    
	if(node->code == BLACK)
	{
		blackNodes.push_back(node);
	}
	else
	{
		for (OctreeNode* child : node->children) {
			getOctreeBlackNodes(child, blackNodes);
		}
	}
}

float getOctreeArea(OctreeNode& node)
{
	std::vector<OctreeNode*> blackNodes{};
	getOctreeBlackNodes(&node,blackNodes);

	const std::vector<glm::vec3> directions = {
		glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), // x-axis
		glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), // y-axis
		glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)  // z-axis
	};

	float total_surface_area = 0.0;

	for (const auto& bNode : blackNodes)
	{
		glm::vec3 w = bNode->posMax.pos - bNode->posMin.pos;
		const float faceArea = w.x * w.x;

		for (const auto& dir : directions) {
			glm::vec3 neighborPosition = node.posMin.pos + dir * faceArea;

			bool neighborFound = false;
			for (const auto& otherBNode : blackNodes) {
				glm::vec3 otherW = otherBNode->posMax.pos - otherBNode->posMin.pos;
				if (neighborPosition == otherBNode->posMin.pos && w == otherW) {
					neighborFound = true;
					break;
				}
			}

			if (!neighborFound) {
				total_surface_area += faceArea;
			}
		}
	}

	return total_surface_area;
}

void buildTreeFromBooleanOperation(const OctreeNode& rootA, const OctreeNode& rootB, OctreeNode& newTree, const Operation& operation, std::string& code, bool bAisNull, bool bBisNull)
{
	if(bAisNull)
	{
		if(rootB.code == GREY)
		{
			newTree.code = GREY;
			code += '(';
			subdivide(newTree);
			for (int i = 0; i < 8; ++i) {
				buildTreeFromBooleanOperation(rootA, *rootB.children[i], *newTree.children[i], operation, code, bAisNull, bBisNull);
			}
			code += ')';
		}
		else
		{
			newTree.code = rootB.code;
			code += codeToChar(rootB.code);
		}
		return;
	}
	
	if(bBisNull)
	{
		if(rootA.code == GREY)
		{
			newTree.code = GREY;
			code += '(';
			subdivide(newTree);
			for (int i = 0; i < 8; ++i) {
				buildTreeFromBooleanOperation(*rootA.children[i], rootB, *newTree.children[i], operation, code, bAisNull, bBisNull);
			}
			code += ')';
		}
		else
		{
			newTree.code = rootA.code;
			code += codeToChar(rootA.code);
		}
		return;
	}
	
	if((rootA.code == WHITE || rootA.code == BLACK) && (rootB.code == WHITE || rootB.code == BLACK))
	{
		if (operation == INTERSECTION) {
			if(rootA.code == rootB.code)
			{
				newTree.code = BLACK;
				code += 'B';
			}
			else
			{
				newTree.code = WHITE;
				code += 'W';
			}
		}
		else if (operation == UNION) {
			if(rootA.code || rootB.code)
			{
				newTree.code = BLACK;
				code += 'B';
			}
			else
			{
				newTree.code = WHITE;
				code += 'W';
			}
		}
	}
	else if(rootA.code == WHITE || rootA.code == BLACK)
	{
		boolOperationOneIsLeaf(rootA,rootB,newTree,operation, code);
	}
	
	else if(rootB.code == WHITE || rootB.code == BLACK)
	{
		boolOperationOneIsLeaf(rootB,rootA,newTree,operation, code);
	}
	if((rootA.code == GREY && rootB.code == GREY) || newTree.code == GREY)
	{
		code += '(';
		subdivide(newTree);
		for (int i = 0; i < 8; ++i) {
			if(rootA.children[i] == nullptr)
				bAisNull = true;
			else
				bAisNull = false;
			if(rootB.children[i] == nullptr)
				bBisNull = true;
			else
				bBisNull = false;
			buildTreeFromBooleanOperation(*rootA.children[i], *rootB.children[i], *newTree.children[i], operation, code, bAisNull, bBisNull);
		}
		code += ')';
	}
}

// void buildTree(MCylinder cylinder ...)
// {
// 	
// }