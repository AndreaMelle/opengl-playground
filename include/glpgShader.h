#ifndef __GLPG_SHADER_H__
#define __GLPG_SHADER_H__

#include "GL/gl3w.h"

namespace glpg
{
    namespace shader
    {
        GLuint fromFile(const char* filename,
                    GLenum shaderType,
#ifdef _DEBUG
                    bool checkErrors = true);
#else
                    bool checkErrors = false);
#endif
        
        GLuint fromString(const char* source,
                    GLenum shaderType,
#ifdef _DEBUG
                    bool checkErrors = true);
#else
                    bool checkErrors = false);
#endif
        
    }
    
    namespace program
    {
        GLuint linkShaders(const GLuint* shaders,
                          int shaderCount,
                          bool deleteShaders,
#ifdef _DEBUG
                          bool checkErrors = true);
#else
                          bool checkErrors = false);
#endif
    }
}

#endif //__GLPG_SHADER_H__