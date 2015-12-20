#ifndef __GLPG_I_MESH_RENDERER_H__
#define __GLPG_I_MESH_RENDERER_H__

#include <vector>
#include <string>
#include <glpgMath.h>

namespace glpg
{
    class IMeshRenderer
    {
    public:
        IMeshRenderer() { }
        virtual ~IMeshRenderer() { }
        
        virtual void render() = 0;
        
    private:
        // Not sure what the copy constructor should be for a MeshRenderer
        // Don't implement it just yet
        IMeshRenderer(const IMeshRenderer& other);
        IMeshRenderer& operator=(const IMeshRenderer& other);
    };
}



#endif // __GLPG_I_MESH_RENDERER_H__
