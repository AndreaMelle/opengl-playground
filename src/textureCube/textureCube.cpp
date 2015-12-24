#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgTextures.h>
#include <iostream>

using namespace glpg;
using namespace glpg::math;

class textureCube : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;
    
    GLuint programID;
    
    struct transform_uniforms_block
    {
        math::mat4 MVP;
    };
    
    GLuint transform_uniforms_buffer;
    GLint texture_sampler_uniform;

    IMeshRenderer* cubeMesh;
    
    GLuint texture;
    
    float aspect;
    math::mat4 projection;
    
    virtual void settings()
    {
        mTitle = "Texture Cube";
        mSamples = 4;
    }
    
    virtual void setup()
    {
        // Set the clear color
        clearColor[0] = 0.5f;
        clearColor[1] = 0.5f;
        clearColor[2] = 0.5f;
        clearColor[3] = 0.0f;
        clearDepth = 1.0f;
        
        // Create and compile shader
        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/unlit/pass.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/unlit/unlit_tex.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        programID = program::linkShaders(shaders, 2, true, true);
        assert(programID != 0);
        texture_sampler_uniform = glGetUniformLocation(programID, "tex");
        
        glUseProgram(programID);
        glUniform1i(texture_sampler_uniform, 0);
        glUseProgram(0);
        
        // Generate the uniform buffer
        glGenBuffers(1, &transform_uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transform_uniforms_block), NULL, GL_DYNAMIC_DRAW);
        
        cubeMesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/cube.obj");
        assert(cubeMesh != NULL);
        
        texture = loadDDS("../models/owl_normals.dds");
        
        
        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        onResize(mWindowWidth, mWindowHeight);
        
    }
    
    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
        
        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);
        
        glUseProgram(programID);
        
        // set the uniforms buffer
        
        math::mat4 model = math::translate(0.0f, -0.2f, -2.0f)
        * math::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
        * math::scale(0.3f);
        math::mat4 MVP = projection * model;
        
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_uniforms_buffer);
        transform_uniforms_block* block = (transform_uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                                      0,
                                                                                      sizeof(transform_uniforms_block),
                                                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        
        block->MVP = MVP;
        
        glUnmapBuffer(GL_UNIFORM_BUFFER);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        cubeMesh->render();
        
        //GLint error = glGetError();
        
    }
    
//    virtual void onKey(const int& key, const int& action)
//    {
//
//    }
    
    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = math::perspective(50.0f, aspect, 0.1f, 1000.0f);
    }
    
    virtual void shutdown()
    {
        delete cubeMesh;
        
        glDeleteProgram(programID);
        glDeleteBuffers(1, &transform_uniforms_buffer);
    }
};

DECLARE_MAIN(textureCube)
