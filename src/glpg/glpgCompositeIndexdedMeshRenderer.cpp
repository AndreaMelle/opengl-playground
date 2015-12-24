#include <glpgCompositeIndexdedMeshRenderer.h>

using namespace glpg;

CompositeIndexedMeshRenderer::CompositeIndexedMeshRenderer()
    : mVao(0)
    , mIsLoaded(false)
    , mHasNormals(false)
    , mHasTangents(false)
{
    for(int i = 0; i < COMPOSITE_INDEXED_MESH_NUM_BUFFERS; ++i)
    {
        mBuffers[i] = 0;
    }
}

CompositeIndexedMeshRenderer::CompositeIndexedMeshRenderer(const std::vector<MeshEntry>& entries,
                                                           const std::vector<math::vec3>& vertices,
                                                           const std::vector<math::vec2>& uvs,
                                                           const std::vector<math::vec3>& normals,
                                                           const std::vector<math::vec3>& tangents,
                                                           const std::vector<unsigned int>& indices)
    : CompositeIndexedMeshRenderer()
{
    this->init(entries, vertices, uvs, normals, tangents, indices);
}

CompositeIndexedMeshRenderer::~CompositeIndexedMeshRenderer()
{
    this->dispose();
}

void CompositeIndexedMeshRenderer::dispose()
{
    // Is it safe or we need to check for 0s?
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(COMPOSITE_INDEXED_MESH_NUM_BUFFERS, mBuffers);

    mVao = 0;
    
    for(int i = 0; i < COMPOSITE_INDEXED_MESH_NUM_BUFFERS; ++i)
    {
        mBuffers[i] = 0;
    }
    
    mEntries.clear();
    
    mIsLoaded = false;
}

void CompositeIndexedMeshRenderer::init(const std::vector<MeshEntry>& entries,
                                        const std::vector<math::vec3>& positions,
                                        const std::vector<math::vec2>& uvs,
                                        const std::vector<math::vec3>& normals,
                                        const std::vector<math::vec3>& tangents,
                                        const std::vector<unsigned int>& indices)
{
    this->dispose();
    
    if(normals.size() > 0) mHasNormals = true;
    if(tangents.size() > 0) mHasTangents = true;
    
    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    glGenBuffers(COMPOSITE_INDEXED_MESH_NUM_BUFFERS, mBuffers);
    
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[INDEX_POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(math::vec3), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOC);
    glVertexAttribPointer(POSITION_LOC, POSITION_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[INDEX_UV_VB]);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(math::vec2), &uvs[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, UV_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    if(mHasNormals)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffers[INDEX_NORMAL_VB]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(math::vec3), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOC);
        glVertexAttribPointer(NORMAL_LOC, NORMAL_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    if(mHasTangents)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffers[INDEX_TANGENT_VB]);
        glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(math::vec3), &tangents[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(TANGENT_LOC);
        glVertexAttribPointer(TANGENT_LOC, TANGENT_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers[INDEX_IB]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    
    mEntries = std::vector<MeshEntry>(entries);
    
    mIsLoaded = true;
}

void CompositeIndexedMeshRenderer::render()
{
    if(!mIsLoaded) return;
    
    glBindVertexArray(mVao);
    
    for (unsigned int i = 0 ; i < mEntries.size() ; i++)
    {
        
        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 mEntries[i].numIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * mEntries[i].baseIndex),
                                 mEntries[i].baseVertex);
    }
    
    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);
}


