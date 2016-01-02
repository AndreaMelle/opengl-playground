#ifndef __GLPG_I_MESH_LOADER_H__
#define __GLPG_I_MESH_LOADER_H__

#include <vector>
#include <string>
#include <glpgMath.h>
#include <glpgIMeshRenderer.h>
#include <limits>

namespace glpg
{
    typedef enum MeshLoaderOptions
    {
        MeshLoaderOption_GenerateNormals    = (1 << 0), // 00000001
        MeshLoaderOption_SmoothNormals      = (1 << 1), // 00000010
        MeshLoaderOption_GenerateTangents   = (1 << 2), // 00000100
        MeshLoaderOption_SetSmoothAngle     = (1 << 4), // 00001000
    } MeshLoaderOptions;
    
    template <class TLoaderImpl, class TMeshInterface>
    class IMeshLoader
    {
    public:
        static TMeshInterface* Create(const std::string& filename,
                                      const unsigned int options = MeshLoaderOption_GenerateNormals | MeshLoaderOption_SmoothNormals,
                                      const float normalSmoothAngle = std::numeric_limits<float>::quiet_NaN())
        {
            return TLoaderImpl::CreateImpl(filename, options, normalSmoothAngle);
        }
    };
}



#endif // __GLPG_I_MESH_LOADER_H__
