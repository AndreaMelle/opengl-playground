#ifndef __GLPG_COMPOSITE_INDEXED_MESH_RENDERER_H__
#define __GLPG_COMPOSITE_INDEXED_MESH_RENDERER_H__

#include "GL/gl3w.h"
#include <glpgMath.h>
#include <glpgIMeshRenderer.h>
#include <vector>

#define COMPOSITE_INDEXED_MESH_NUM_BUFFERS 5

namespace glpg
{
    class CompositeIndexedMeshRenderer : public IMeshRenderer
    {
    public:
        struct MeshEntry
        {
            MeshEntry()
            {
                numIndices = 0;
                baseVertex = 0;
                baseIndex = 0;
                materialIndex = INVALID_MATERIAL;
            }
            
            unsigned int numIndices;
            unsigned int baseVertex;
            unsigned int baseIndex;
            unsigned int materialIndex;
        };
        
        CompositeIndexedMeshRenderer(const std::vector<MeshEntry>& entries,
                                     const std::vector<math::vec3>& vertices,
                                     const std::vector<math::vec2>& uvs,
                                     const std::vector<math::vec3>& normals,
                                     const std::vector<math::vec3>& tangents,
                                     const std::vector<unsigned int>& indices);
        
        
        virtual ~CompositeIndexedMeshRenderer();

        virtual void render();

    private:
        
        void init(const std::vector<MeshEntry>& entries,
                  const std::vector<math::vec3>& vertices,
                  const std::vector<math::vec2>& uvs,
                  const std::vector<math::vec3>& normals,
                  const std::vector<math::vec3>& tangents,
                  const std::vector<unsigned int>& indices);
        
        void dispose();
        
        // Locations in program for attributes
        static const int POSITION_LOC       = 0;
        static const int UV_LOC             = 1;
        static const int NORMAL_LOC         = 2;
        static const int TANGENT_LOC        = 3;
        
        // Index in buffers
        static const int INDEX_IB           = 0;
        static const int INDEX_POS_VB       = 1;
        static const int INDEX_NORMAL_VB    = 2;
        static const int INDEX_UV_VB        = 3;
        static const int INDEX_TANGENT_VB   = 4;
        
        // Sizes of stuff
        static const int POSITION_SIZE      = 3;
        static const int UV_SIZE            = 2;
        static const int NORMAL_SIZE        = 3;
        static const int TANGENT_SIZE       = 3;
        
        // But we don't support indexed materials just yet
        static const unsigned int INVALID_MATERIAL = 0xFFFFFFFF;
      
        GLuint mVao;
        GLuint mBuffers[COMPOSITE_INDEXED_MESH_NUM_BUFFERS];
        
        std::vector<MeshEntry> mEntries;
        
        bool mIsLoaded;
        
        bool mHasNormals;
        bool mHasTangents;

    private:
        CompositeIndexedMeshRenderer();
        
        // Not sure what the copy constructor should be for a MeshRenderer
        // Don't implement it just yet
        CompositeIndexedMeshRenderer(const CompositeIndexedMeshRenderer& other);
        CompositeIndexedMeshRenderer& operator=(const CompositeIndexedMeshRenderer& other);

    };
}

#endif //__GLPG_COMPOSITE_INDEXED_MESH_RENDERER_H__
