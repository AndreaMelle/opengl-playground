#ifndef __GLPG_I_MESH_LOADER_H__
#define __GLPG_I_MESH_LOADER_H__

#include <vector>
#include <string>
#include <glpgMath.h>
#include <glpgIMeshRenderer.h>

namespace glpg
{
    template <class TLoaderImpl, class TMeshInterface>
    class IMeshLoader
    {
    public:
        static TMeshInterface* Create(const std::string& filename)
        {
            return TLoaderImpl::CreateImpl(filename);
        }
    };
}



#endif // __GLPG_I_MESH_LOADER_H__
