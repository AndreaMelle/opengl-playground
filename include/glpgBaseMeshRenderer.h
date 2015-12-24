#ifndef __GLPG_BASE_MESH_RENDERER_H__
#define __GLPG_BASE_MESH_RENDERER_H__

#include "GL/gl3w.h"
#include <glpgMath.h>
#include <glpgIMeshRenderer.h>
#include <vector>

namespace glpg
{
    class BaseMeshRenderer : public IMeshRenderer
    {
    public:
        BaseMeshRenderer(const int size,
                         const int offset,
                         const std::vector<math::vec3>& vertices,
                         const std::vector<math::vec2>& uvs,
                         const std::vector<math::vec3>& normals,
                         const GLenum mode = GL_TRIANGLES,
                         const std::vector<math::vec3>& colors = std::vector<math::vec3>());
        
        
        virtual ~BaseMeshRenderer();

        virtual void render();

    private:
        
        void init(const int& size,
                  const int& offset,
                  const std::vector<math::vec3>& vertices,
                  const std::vector<math::vec2>& uvs,
                  const std::vector<math::vec3>& normals,
                  const GLenum mode,
                  const std::vector<math::vec3>& colors = std::vector<math::vec3>());
        
        void dispose();

        static const int POSITION_LOC = 0;
        static const int UV_LOC = 1;
        static const int NORMAL_LOC = 2;
        static const int COLOR_LOC = 3;
        
        static const int POSITION_SIZE = 3;
        static const int UV_SIZE = 2;
        static const int NORMAL_SIZE = 3;
        static const int COLOR_SIZE = 3;
      
        GLuint vao;

        GLuint vertexBuffer;
        GLuint uvBuffer;
        GLuint normalBuffer;
        GLuint colorBuffer;
        
        int mVertexCount;
        
        bool mIsLoaded;
        
        GLenum drawMode;

    private:
        BaseMeshRenderer();
        
        // Not sure what the copy constructor should be for a MeshRenderer
        // Don't implement it just yet
        BaseMeshRenderer(const BaseMeshRenderer& other);
        BaseMeshRenderer& operator=(const BaseMeshRenderer& other);

    };
}

#endif //__GLPG_MESH_RENDERER_H__
