#include <glpgAssImpLoader.h>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

glpg::CompositeIndexedMeshRenderer* glpg::AssImpLoader::CreateImpl(const std::string& filename)
{
    
    Assimp::Importer importer;
    
    //importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 35.0f);
    
    const aiScene* scene = importer.ReadFile(filename.c_str()
                                             , aiProcess_Triangulate
                                             | aiProcess_GenSmoothNormals
                                             | aiProcess_FlipUVs
                                             | aiProcess_FindDegenerates
                                             | aiProcess_FixInfacingNormals
                                             | aiProcess_FindInvalidData
                                             );
    
    if (!scene)
    {
        printf("Error parsing file %s : %s\n", filename.c_str(), importer.GetErrorString());
        return NULL;
    }
    
    std::vector<glpg::CompositeIndexedMeshRenderer::MeshEntry> entries;
    
    entries.resize(scene->mNumMeshes);
    
    std::vector<math::vec3> positions;
    std::vector<math::vec2> uvs;
    std::vector<math::vec3> normals;
    std::vector<unsigned int> indices;
    
    unsigned int numVertices = 0;
    unsigned int numIndices = 0;
    
    // Count the number of vertices and indices
    for (unsigned int i = 0 ; i < entries.size() ; i++)
    {
        entries[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;
        entries[i].numIndices = scene->mMeshes[i]->mNumFaces * 3;
        entries[i].baseVertex = numVertices;
        entries[i].baseIndex = numIndices;
        
        numVertices += scene->mMeshes[i]->mNumVertices;
        numIndices  += entries[i].numIndices;
    }
    
    // Reserve space in the vectors for the vertex attributes and indices
    positions.reserve(numVertices);
    normals.reserve(numVertices);
    uvs.reserve(numVertices);
    indices.reserve(numIndices);
    
    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0 ; i < entries.size() ; i++)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        
        // Populate the vertex attribute vectors
        for (unsigned int j = 0 ; j < mesh->mNumVertices ; j++)
        {
            const aiVector3D* pPos      = &(mesh->mVertices[j]);
            const aiVector3D* pNormal   = &(mesh->mNormals[j]);
            const aiVector3D* pTexCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &Zero3D;
            
            positions.push_back(math::vec3(pPos->x, pPos->y, pPos->z));
            normals.push_back(math::vec3(pNormal->x, pNormal->y, pNormal->z));
            uvs.push_back(math::vec2(pTexCoord->x, pTexCoord->y));
        }
        
        // Populate the index buffer
        for (unsigned int j = 0; j < mesh->mNumFaces ; j++)
        {
            const aiFace& Face = mesh->mFaces[j];
            assert(Face.mNumIndices == 3);
            indices.push_back(Face.mIndices[0]);
            indices.push_back(Face.mIndices[1]);
            indices.push_back(Face.mIndices[2]);
        }
        
        
    }
    
    
    glpg::CompositeIndexedMeshRenderer* mesh = new glpg::CompositeIndexedMeshRenderer(entries,
                                                                                      positions,
                                                                                      uvs,
                                                                                      normals,
                                                                                      indices);
    
    return mesh;
}