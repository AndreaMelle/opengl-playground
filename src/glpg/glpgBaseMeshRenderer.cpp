#include <glpgBaseMeshRenderer.h>

using namespace glpg;

BaseMeshRenderer::BaseMeshRenderer()
    : vao(0)
    , vertexBuffer(0)
    , uvBuffer(0)
    , normalBuffer(0)
    , colorBuffer(0)
    , mIsLoaded(false)
    , drawMode(GL_TRIANGLES) { }

BaseMeshRenderer::BaseMeshRenderer(const int size,
                                   const int offset,
                                   const std::vector<math::vec3>& vertices,
                                   const std::vector<math::vec2>& uvs,
                                   const std::vector<math::vec3>& normals,
                                   const GLenum mode,
                                   const std::vector<math::vec3>& colors)
    : BaseMeshRenderer()
{
    this->init(size, offset, vertices, uvs, normals, mode, colors);
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
    glDeleteBuffers(1, &colorBuffer);

    vao = 0;
    vertexBuffer = 0;
    uvBuffer = 0;
    normalBuffer = 0;
    colorBuffer = 0;
    
    mIsLoaded = false;
}

void BaseMeshRenderer::init(const int& size,
                            const int& offset,
                            const std::vector<math::vec3>& vertices,
                            const std::vector<math::vec2>& uvs,
                            const std::vector<math::vec3>& normals,
                            const GLenum mode,
                            const std::vector<math::vec3>& colors)
{
    this->dispose();
    
    //No index support
    
    mVertexCount = size;
    drawMode = mode;
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(vertices.size() > 0)
    {
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec3), &vertices[offset], GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION_LOC);
        glVertexAttribPointer(POSITION_LOC, POSITION_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    if(uvs.size() > 0)
    {
        glGenBuffers(1, &uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec2), &uvs[offset], GL_STATIC_DRAW);
        glEnableVertexAttribArray(UV_LOC);
        glVertexAttribPointer(UV_LOC, UV_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    if(normals.size() > 0)
    {
        glGenBuffers(1, &normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec3), &normals[offset], GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOC);
        glVertexAttribPointer(NORMAL_LOC, NORMAL_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    if(colors.size() > 0)
    {
        glGenBuffers(1, &colorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertexCount * sizeof(math::vec3), &colors[offset], GL_STATIC_DRAW);
        glEnableVertexAttribArray(COLOR_LOC);
        glVertexAttribPointer(COLOR_LOC, COLOR_SIZE, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
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


