#include "MOctree.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION

#include "MRenderer.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void printV3(Vertex v)
{
    std::cout << v.pos.x << " " << v.pos.y << " " << v.pos.z;
}

void transverseTree(OctreeNode& oct)
{
    std::cout << "Pos Min: ";
    printV3(oct.posMin);
    std::cout <<" Pos Max: ";
    printV3(oct.posMax);
    std::cout << " Code: " << oct.code << std::endl;
    
    for(auto o : oct.children)
    {
        if(o != nullptr)
        {
            transverseTree(*o);
        }
    }
}

std::string getInputCode()
{
    std::string inputCode = "";
    std::ifstream inputFile;
    inputFile.open("IO/input.txt", std::ios_base::in | std::ios_base::binary);
    if (!inputFile) {
        std::cerr << "Error opening input file!" << std::endl;
    }
    else
    {
        std::getline(inputFile, inputCode);
    }

    inputFile.close();
    return inputCode;
}

int main(int argc, char* argv[])
{
    MModel m;
    std::string code;
    std::string boolCode = "";
    
    m.loadModel("models/animals/PSX_shark.obj");
    //m.loadModel("models/Car/Datsun_280Z.obj");
    
    OctreeNode Occ{};
    OctreeNode Occ2{};
    OctreeNode boolOpTree{};
    boolOpTree.posMin.pos = glm::vec3(-5.0f, -5.0f, -5.0f);
    boolOpTree.posMax.pos = glm::vec3(5.0f, 5.0f, 5.0f);
    
    bool bBuildModel = false;
    bool bBuildSphere = false;
    bool bBuildBlock = true;
    bool bBuildCylinder = false;
    Operation OP = UNION;
    bool bInput = false;
    bool bShowModel = false;
    bool bBoolOperation = false;
    bool bTranslate = false;
    bool bScale = false;
    glm::vec3 translationVector = glm::vec3{3.f,3.f,3.f};
    float scalar = 3.0f;

    bool bCalculateVolume = true;
    bool bCalculateArea = false;
    uint8_t depth = 8;
    
    Sphere sphere = {{glm::vec3(0.0f, 0.0f, 0.0f)}, 3.0f};
    Block block = {{glm::vec3(1.0f, 1.0f, 1.0f)},glm::vec3(5.0f, 2.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f)};
    Cylinder cylinder = {{glm::vec3(-2.0f, -2.0f, -2.0f)},10.0f, 5.0f, glm::vec3(0.0f, 0.0f, 0.0f)};
    Cone cone = {{glm::vec3(3.0f, 3.0f, 3.0f)},2.5f, 6.0f, glm::vec3(0.0f, 0.0f, 0.0f)};
    
    
    if(bInput)
    {
        code = getInputCode();
        Occ = buildTreeFromCode(code);
    }
    else
    {
        if(bBuildModel)
        {
            Occ = buildInitialBoundingBox(m);
            buildTree(cylinder, Occ, depth, code);
        }
        else if(bBuildBlock)
        {
            Occ = createNodeForBlock(block);
            buildTree(cylinder, Occ, depth, code);
        }
        else if(bBuildSphere)
        {
            Occ = createNodeForSphere(sphere);
            buildTree(cylinder, Occ, depth, code);
        }
        else if(bBuildCylinder)
        {
            Occ =  createNodeForCylinder(cylinder);
            buildTree(cylinder, Occ, depth, code);
        }
        else
        {
            Occ = createNodeForCone(cone);
            buildTree(cylinder, Occ, depth, code);
        }
    }

    
    if(bTranslate)
    {
        Occ = buildTreeFromCode(code, Occ.posMin.pos + translationVector, Occ.posMax.pos + translationVector);
    }

    if(bScale)
    {
        Occ = buildTreeFromCode(code, Occ.posMin.pos * scalar, Occ.posMax.pos * scalar);
    }
    
    std::ofstream outputFile("IO/output.txt");
    if (!outputFile)
    {
        std::cerr << "Error opening output file!" << std::endl;
    }
    else
    {
        outputFile << code;
        outputFile.close();
    }

    if(bBoolOperation)
    {
        std::string code2 = "((WWW(WW(WBWBBBBB)(BBBBBBWW)(WWBBWWBB)W(BBBBBBBB)(BBBWBBBW))WWW((WWBBWWWW)W(BBBBWBWB)(BBBWBBWW)WW(WBWBWBWB)(BBWWBBWW)))(WW(WW(BBBBBBWW)(BWBWBBBB)W(WWBBWWBB)(BBWBBBWB)(BBBBBBBB))WWW(W(WWBBWWWW)(BBWBBBWW)(BBBBBWBW)WW(BBWWBBWW)(BWBWBWBW))W)(W((WBWWWBWB)(BBWWBBBB)WW(BBWBBBWB)(BBBBBBBB)W(WWWWBBWW))WWW((BBWBWBWB)(BBBBBBBB)(WBWWWBWW)(BBWWBBWW)(WBWBBBBB)(BBBBBBBB)(WBWWWWWW)(BBWWBBWW))WW)(((BBWWBBBB)(BWWWBWBW)WW(BBBBBBBB)(BBBWBBBW)(WWWWBBWW)W)WWW((BBBBBBBB)(BBBWBWBW)(BBWWBBWW)(BWWWBWWW)(BBWBBBBB)(BWBWBBBB)(BBWWBBWW)(BWWWWWWW))WWW)(WWW(WW(BBBBBBBB)(BBWWBBWW)(WWWWWWBB)W(BBBBBBBB)(BBBWBBBW))WWW((WWBBWWWW)W(BBBBBBBB)(BBBWBBBW)WW(WBWBWBWB)(BBBBBBBB)))(WW(WW(BBWWBBWW)(BBBBBBBB)W(WWWWWWBB)(BBWBBBWB)(BBBBBBBB))WWW(W(WWBBWWWW)(BBWBBBWB)(BBBBBBBB)WW(BBBBBBBB)(BWBWBWBW))W)(W((BBWBBBWW)(BBBBBBWB)WW(BBWWBBWW)(BBWWBBWW)WW)WWW((BBWWWBWW)(BBWWBBWW)WWW(BBWWWWWW)WW)WW)(((BBBBBBWW)(BBBWBBWW)WW(BBWWBBWW)(BBWWBBWW)WW)WWW((BBWWBBWW)(BBWWBWWW)WW(BBWWWWWW)WWW)WWW))";
        Occ2 = buildTreeFromCode(code2);
        buildTreeFromBooleanOperation(Occ, Occ2, boolOpTree, UNION, boolCode);
        boolOpTree = buildTreeFromCode(boolCode);
        std::cout << boolCode << std::endl;
    }
    
    m.currentIndex++;
    if(!bShowModel)
    {
        m.vertices.clear();
        m.indices.clear();
        m.currentIndex = 0;
    }
    
    if(bCalculateVolume)
        std::cout << "Volume: " << getOctreeVolume(&Occ) << std::endl;
    if(bCalculateArea)
        std::cout << "Area: " << getOctreeArea(Occ) << std::endl;

    populateFromOctree(&Occ, m.vertices, m.indices, m.currentIndex);
    
    MRenderer program;
    program.run(m.vertices, m.indices);
    
    return 0;
}
