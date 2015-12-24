#include <glpgBaseMeshLoader.h>
#include <string>

//FIXME: no indices support. This will generate triangles for everything
glpg::BaseMeshRenderer* glpg::BaseMeshLoader::CreateImpl(const std::string& filename,
                                                         const unsigned int options,
                                                         const float normalSmoothAngle)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    
    std::vector<math::vec3> tempVertices;
    std::vector<math::vec2> tempUVs;
    std::vector<math::vec3> tempNormals;
    
    std::vector<math::vec3> outVertices;
    std::vector<math::vec2> outUVs;
    std::vector<math::vec3> outNormals;
    
    FILE* file = fopen(filename.c_str(), "r");
    
    if(file == NULL)
    {
        printf("Failed to open file: %s\n", filename.c_str());
        return NULL;
    }
    
    bool complete = false;
    
    while (1)
    {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if(res == EOF) break;
        
        if (strcmp(lineHeader, "v") == 0)
        {
            math::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex[0], &vertex[1], &vertex[2]);
            tempVertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            math::vec2 uv;
            fscanf(file, "%f %f\n", &uv[0], &uv[1]);
            tempUVs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            math::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal[0], &normal[1], &normal[2]);
            tempNormals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            //std::string v1, v2, v3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if(matches == 9)
            {
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                
                uvIndices.push_back(uvIndex[0]);
                uvIndices.push_back(uvIndex[1]);
                uvIndices.push_back(uvIndex[2]);
                
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
                
                complete = true;
                
            }
            else
            {
                matches = fscanf(file, "%d %d\n", &vertexIndex[1], &vertexIndex[2]);
                if(matches == 2)
                {
                    vertexIndices.push_back(vertexIndex[0]);
                    vertexIndices.push_back(vertexIndex[1]);
                    vertexIndices.push_back(vertexIndex[2]);
                    
                    complete = false;
                }
                else
                {
                    printf("OBJ format error.\n");
                    return NULL;
                }
            }
            
        }
        else
        {
            // Ok, but this technique fails if a line is larger than 1024 bytes??
            char buffer[1024];
            fgets(buffer, 1024, file);
        }
        
    }
    
    for(unsigned int i = 0; i < vertexIndices.size(); ++i)
    {
        unsigned int vertexIndex = vertexIndices[i];
        math::vec3 vertex = tempVertices[vertexIndex - 1];
        outVertices.push_back(vertex);
        
        unsigned int uvIndex;
        unsigned int normalIndex;
        
        if(complete)
        {
            uvIndex = uvIndices[i];
            normalIndex = normalIndices[i];
        }
        else
        {
            uvIndex = vertexIndex;
            normalIndex = vertexIndex;
        }
        
        if(tempUVs.size() > (uvIndex - 1))
        {
            math::vec2 uv = tempUVs[uvIndex - 1];
            outUVs.push_back(uv);
        }
        
        if(tempNormals.size() > (normalIndex - 1))
        {
            math::vec3 normal = tempNormals[normalIndex - 1];
            outNormals.push_back(normal);
        }
    }
    
    BaseMeshRenderer* mesh = new BaseMeshRenderer(outVertices.size(),
                                                  0,
                                                  outVertices,
                                                  outUVs,
                                                  outNormals);
    
    return mesh;
}