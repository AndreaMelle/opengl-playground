#ifndef __GLPG_ASSIMP_LOADER_H__
#define __GLPG_ASSIMP_LOADER_H__

#include <vector>
#include <glpgMath.h>
#include <glpgIMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>

namespace glpg
{
    class AssImpLoader : public IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>
    {
    public:
        static CompositeIndexedMeshRenderer* CreateImpl(const std::string& filename,
                                                        const unsigned int options = MeshLoaderOption_GenerateNormals | MeshLoaderOption_SmoothNormals,
                                                        const float normalSmoothAngle = std::numeric_limits<float>::quiet_NaN());
        
    private:
        AssImpLoader() {}
        
    };
}



#endif // __GLPG_ASSIMP_LOADER_H__
