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
        static CompositeIndexedMeshRenderer* CreateImpl(const std::string& filename);
        
    private:
        AssImpLoader() {}
        
    };
}



#endif // __GLPG_ASSIMP_LOADER_H__
