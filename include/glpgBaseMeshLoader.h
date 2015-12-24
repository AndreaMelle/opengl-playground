#ifndef __GLPG_BASE_MESH_LOADER_H__
#define __GLPG_BASE_MESH_LOADER_H__

#include <vector>
#include <glpgMath.h>
#include <glpgIMeshLoader.h>
#include <glpgBaseMeshRenderer.h>

namespace glpg
{
    class BaseMeshLoader : public IMeshLoader<BaseMeshLoader, BaseMeshRenderer>
    {
    public:
        static BaseMeshRenderer* CreateImpl(const std::string& filename,
                                            const unsigned int options = MeshLoaderOption_GenerateNormals | MeshLoaderOption_SmoothNormals,
                                            const float normalSmoothAngle = std::numeric_limits<float>::quiet_NaN());
    private:
        BaseMeshLoader() {}
    };
}



#endif // __GLPG_BASE_MESH_LOADER_H__
