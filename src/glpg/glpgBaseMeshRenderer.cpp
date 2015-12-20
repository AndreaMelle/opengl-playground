#include <glpgBaseMeshRenderer.h>

using namespace glpg;

BaseMeshRenderer::BaseMeshRenderer()
    : vao(0)
    , vertexBuffer(0)
    , uvBuffer(0)
    , normalBuffer(0)
    , mIsLoaded(false)
    , drawMode(GL_TRIANGLES) { }

BaseMeshRenderer::BaseMeshRenderer(const int& size,
                                   const int& offset,
                                   const std::vector<math::vec3>& vertices,
                                   const std::vector<math::vec2>& uvs,
                                   const std::vector<math::vec3>& normals,
                                   const GLenum mode)
    : BaseMeshRenderer()
{
    this->init(size, offset, vertices, uvs, normals, mode);
}

BaseMeshRenderer::~BaseMeshRenderer()
{
    this->dispose();
}

void BaseMeshRenderer::dispose()
{
    // Is it safe or we need to check for 0s?
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &normalBuffer);

    vao = 0;
    vertexBuffer = 0;
    uvBuffer = 0;
    normalBuffer = 0;
    
    mIsLoaded = false;
}

void BaseMeshRenderer::init(const int& size,
                            const int& offset,
                            const std::vector<math::vec3>& vertices,
                            const std::vector<math::vec2>& uvs,
                            const std::vector<math::vec3>& normals,
                            const GLenum mode)
{
    this->dispose();
    
    //No index support
    
    mVertexCount = size;
    drawMode = mode;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec3), &vertices[offset], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOC);
    glVertexAttribPointer(POSITION_LOC, POSITION_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec2), &uvs[offset], GL_STATIC_DRAW);
    glEnableVertexAttribArray(UV_LOC);
    glVertexAttribPointer(UV_LOC, UV_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec3), &normals[offset], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOC);
    glVertexAttribPointer(NORMAL_LOC, NORMAL_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
    
    mIsLoaded = true;
}

void BaseMeshRenderer::render()
{
    if(!mIsLoaded) return;
    
    glBindVertexArray(vao);
    glDrawArrays(drawMode, 0, mVertexCount);
    glBindVertexArray(0);
}


