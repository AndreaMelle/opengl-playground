#define _CRT_SECURE_NO_WARNINGS

#include <glpgShader.h>

#include <cstdio>

using namespace glpg;
using namespace glpg::shader;
using namespace glpg::program;

namespace glpg
{
    namespace shader
    {
        extern
        GLuint fromFile(const char* filename, GLenum shaderType, bool checkErrors)
        {
            GLuint result = 0;
            FILE* fp;
            size_t filesize;
            char* data;
            
            fp = fopen(filename, "rb");
            
            if(!fp) return 0;
            
            // Get size of file by seeking to the end and read index
            fseek(fp, 0, SEEK_END);
            filesize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            
            data = new char[filesize + 1];
            
            if(!data)
                goto fail_data_alloc;
            
            // read all data one byte at a time, and set termination flag
            fread(data, 1, filesize, fp);
            data[filesize] = 0;
            fclose(fp);
            
            // create a shader
            result = glCreateShader(shaderType);
            
            if(!result)
                goto fail_shader_alloc;
            
            // set the shader source
            glShaderSource(result, 1, &data, NULL);
            
            delete [] data;
            
            
            // compile the shader
            glCompileShader(result);
            
            if(checkErrors)
            {
                GLint status = 0;
                glGetShaderiv(result, GL_COMPILE_STATUS, &status);
                
                if(!status)
                {
                    char buffer[4096];
                    glGetShaderInfoLog(result, 4096, NULL, buffer);
#ifdef _WIN32
                    OutputDebugStringA(filename);
                    OutputDebugStringA(":");
                    OutputDebugStringA(buffer);
                    OutputDebugStringA("\n");
#else
                    fprintf(stderr, "%s: %s\n", filename, buffer);
#endif
                    goto fail_compile_shader;
                }
            }
            
            return result;
            
        fail_compile_shader:
            glDeleteShader(result);
            
        fail_shader_alloc:;
        fail_data_alloc:
            return result;
        }
        
        extern
        GLuint fromString(const char* source, GLenum shaderType, bool checkErrors)
        {
            GLuint result;
            
            result = glCreateShader(shaderType);
            
            const char * strings[] = { source };
            glShaderSource(result, 1, strings, NULL);
            
            glCompileShader(result);
            
            if (checkErrors)
            {
                GLint status = 0;
                glGetShaderiv(result, GL_COMPILE_STATUS, &status);
                
                if (!status)
                {
                    char buffer[4096];
                    glGetShaderInfoLog(result, 4096, NULL, buffer);
#ifdef _WIN32
                    OutputDebugStringA(buffer);
                    OutputDebugStringA("\n");
#else
                    fprintf(stderr, "%s\n", buffer);
#endif
                    goto fail_compile_shader;
                }
            }
            
            return result;
            
        fail_compile_shader:
            glDeleteShader(result);
            
            return 0;
        }
    }
    
    namespace program
    {
        extern
        GLuint linkShaders(const GLuint* shaders, int shaderCount, bool deleteShaders, bool checkErrors)
        {
            int i;
            GLuint program;
            
            program = glCreateProgram();
            
            // Attach all the shaders
            for (i = 0; i < shaderCount; ++i)
            {
                glAttachShader(program, shaders[i]);
            }
            
            glLinkProgram(program);
            
            if (checkErrors)
            {
                GLint status;
                glGetProgramiv(program, GL_LINK_STATUS, &status);
                
                if (!status)
                {
                    char buffer[4096];
                    glGetProgramInfoLog(program, 4096, NULL, buffer);
#ifdef _WIN32
                    OutputDebugStringA(buffer);
                    OutputDebugStringA("\n");
#endif
                    glDeleteProgram(program);
                    return 0;
                }
            }
            
            // Once compiled the program, we can delete the shaders
            // This is optional, as the shaders could be reused for a different program
            if(deleteShaders)
            {
                for (i = 0; i < shaderCount; ++i)
                {
                    glDeleteShader(shaders[i]);
                }
            }
            
            return program;
            
        }
    }
}



