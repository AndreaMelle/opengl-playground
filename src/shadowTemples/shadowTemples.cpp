#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgPrimitives.h>
#include <glpgArcball.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_COMPARE_R_TO_TEXTURE           0x884E

using namespace glpg;
using namespace glpg::math;

class shadowTemples : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;

    struct transform_uniforms_block
    {
        glm::mat4 modelview;
        glm::mat4 MVP;
        glm::mat4 normalmatrix;
        glm::mat4 shadowMVP;
    };

    GLuint transform_uniforms_buffer;

    struct
    {
        glm::vec4 light_pos;
        GLint u_light_pos; //vec4
        GLint u_light_intensity; //vec3
        GLint u_attenuation; //vec3 => Constant, Linear, Exp
    } lightParams;

    struct
    {
        GLuint progID;
        GLint u_ambient_color; //vec3
        GLint u_diffuse_color; //vec3
        GLint u_specular_color; //vec3
        GLint u_specular_power; //float
        GLint u_rendermode;
    } matParams;
    
    struct
    {
        GLint u_shadowmap;
        GLint u_bias;
        float bias;
    } shadowParams;
    
    struct
    {
        GLuint progID;
        GLint u_MVP;
        GLuint framebuffer;
        GLuint depthbuffer;
        glm::mat4 depthProj;
    } depthPass;
    
    struct
    {
        GLuint progID;
        GLint u_depthbuffer;
        GLuint fullscreenQuadVao;
    } depthViz;
    
    struct
    {
        IMeshRenderer* mesh;
        glm::mat4 transform;
    } temple;

    float aspect;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 root;
    
    Arcball* arcball;
    
    glm::mat4 bias;
    
    enum
    {
        RENDER_FULL = 1,
        RENDER_SHADOWMAP,
        RENDER_NOSHADOWS,
        RENDER_SHADOWSONLY
    } rendermode;

    virtual void settings()
    {
        mTitle = "BlinnPhong Teapot";
    }

    virtual void setup()
    {
        clearColor[0] = 0.5f; clearColor[1] = 0.5f; clearColor[2] = 0.5f; clearColor[3] = 0.0f;
        clearDepth = 1.0f;
        
        bias = glm::mat4 (0.5, 0.0, 0.0, 0.0,
                          0.0, 0.5, 0.0, 0.0,
                          0.0, 0.0, 0.5, 0.0,
                          0.5, 0.5, 0.5, 1.0);
        
        //view = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.5f, -4.0f));
        view = glm::lookAt(glm::vec3(0.0f, 2.5f, -5.0f), glm::vec3(0,0,0), glm::vec3(0, 1.0, 0));
        lightParams.light_pos = glm::vec4(glm::normalize(glm::vec3(0.5, -0.5, 0)), 0.0);
        
        depthPass.depthProj = glm::ortho<float>(-5,10,-5,5,-10,20);
        
        depthPass.framebuffer = 0;
        glGenFramebuffers(1, &depthPass.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, depthPass.framebuffer);
        
        glGenTextures(1, &depthPass.depthbuffer);
        glBindTexture(GL_TEXTURE_2D, depthPass.depthbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthPass.depthbuffer, 0);
        glDrawBuffer(GL_NONE); // No color buffer is drawn to.
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/phong/phong_shadowmap.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/phong/phong_shadowmap.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        matParams.progID = program::linkShaders(shaders, 2, true, true);
        assert(matParams.progID != 0);
        lightParams.u_light_pos = glGetUniformLocation(matParams.progID, "light_pos");
        lightParams.u_light_intensity = glGetUniformLocation(matParams.progID, "lightParams.light_intensity");
        lightParams.u_attenuation = glGetUniformLocation(matParams.progID, "lightParams.attenuation");
        matParams.u_ambient_color = glGetUniformLocation(matParams.progID, "matParams.ambient_color");
        matParams.u_diffuse_color = glGetUniformLocation(matParams.progID, "matParams.diffuse_color");
        matParams.u_specular_color = glGetUniformLocation(matParams.progID, "matParams.specular_color");
        matParams.u_specular_power = glGetUniformLocation(matParams.progID, "matParams.specular_power");
        shadowParams.u_shadowmap = glGetUniformLocation(matParams.progID, "shadowParams.shadowmap");
        shadowParams.u_bias = glGetUniformLocation(matParams.progID, "shadowParams.bias");
        matParams.u_rendermode = glGetUniformLocation(matParams.progID, "mode");
        
        shaders[0] = shader::fromFile("../shaders/depthpass/depthpass.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/depthpass/depthpass.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        depthPass.progID = program::linkShaders(shaders, 2, true, true);
        assert(depthPass.progID != 0);
        depthPass.u_MVP = glGetUniformLocation(depthPass.progID, "MVP");
        
        shaders[0] = shader::fromFile("../shaders/depthpass/quad.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/depthpass/depthviz.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        depthViz.progID = program::linkShaders(shaders, 2, true, true);
        assert(depthViz.progID != 0);
        depthViz.u_depthbuffer = glGetUniformLocation(depthViz.progID, "depth_buffer");

        glUseProgram(matParams.progID);
        glUniform3fv(matParams.u_ambient_color, 1, math::vec3(0.0));
        glUniform3fv(matParams.u_diffuse_color, 1, math::vec3(1.0, 1.0, 1.0));
        glUniform3fv(matParams.u_specular_color, 1, math::vec3(1.0, 1.0, 1.0));
        glUniform1f(matParams.u_specular_power, 128.0f);
        glm::vec4 lightEyeSpace = view * lightParams.light_pos;
        lightEyeSpace = glm::vec4(lightEyeSpace.x, lightEyeSpace.y, lightEyeSpace.z, 0.0);
        glUniform4fv(lightParams.u_light_pos, 1, glm::value_ptr(lightEyeSpace));
        glUniform3fv(lightParams.u_light_intensity, 1, math::vec3(1.0));
        glUniform1i(shadowParams.u_shadowmap, 0);
        shadowParams.bias = 0.005f;
        glUniform1f(shadowParams.u_bias, shadowParams.bias);
        glUseProgram(0);
        
        glUseProgram(depthViz.progID);
        glUniform1i(depthViz.u_depthbuffer, 0);
        glUseProgram(0);
        
        glGenBuffers(1, &transform_uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transform_uniforms_block), NULL, GL_DYNAMIC_DRAW);
        
        temple.mesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/temples.obj");
        assert(temple.mesh != NULL);
        temple.transform = glm::scale(glm::mat4(), glm::vec3(0.5f));
        
        glGenVertexArrays(1, &depthViz.fullscreenQuadVao);
        glBindVertexArray(depthViz.fullscreenQuadVao);
        
        arcball = new Arcball(mWindowWidth, mWindowHeight);
        root = glm::mat4();

        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        rendermode = RENDER_FULL;
        
        
        onResize(mWindowWidth, mWindowHeight);
    }
    
    virtual void render(double currentTime)
    {
        //arcball->update(view, root);
        
        glm::mat4 depthView = glm::lookAt(-glm::vec3(lightParams.light_pos), glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 depthMVP = depthPass.depthProj * depthView * root * temple.transform;
        glm::mat4 shadowMVP = bias * depthMVP;
        
        //bind depth pass framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthPass.framebuffer);
        glViewport(0, 0, 1024, 1024);
        glDrawBuffer(GL_NONE);
        
        // clear it
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);
        
        //render scene
        glUseProgram(depthPass.progID);
        
        glUniformMatrix4fv(depthPass.u_MVP, 1, GL_FALSE,glm::value_ptr(depthMVP));
        
        // We can't use culling because the mesh we are using is not really shadowmap friendly.
        // The thin border at the top of the temple's roof messes up with the edge right below when only backfaces are rendered
        glDisable(GL_CULL_FACE);
        //glCullFace(GL_FRONT); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        temple.mesh->render();
        glUseProgram(0);
        
        //bind regular framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        if(rendermode == RENDER_SHADOWMAP)
        {
            glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
            glDrawBuffer(GL_BACK); //we set back buffer again
            
            // Bind the freshly filled render textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthPass.depthbuffer);
            
            glUseProgram(depthViz.progID);
            
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
            
            glBindVertexArray(depthViz.fullscreenQuadVao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
            glDrawBuffer(GL_BACK);
            
            glClearBufferfv(GL_COLOR, 0, clearColor);
            glClearBufferfv(GL_DEPTH, 0, &clearDepth);
            
            glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
            glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
            
            glm::mat4 MVP = projection * view * root * temple.transform;
            glm::mat4 modelview = view * root * temple.transform;
            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelview));
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthPass.depthbuffer);
            
            glUseProgram(matParams.progID);
            
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_uniforms_buffer);
            transform_uniforms_block* block = (transform_uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                                          0,
                                                                                          sizeof(transform_uniforms_block),
                                                                                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            
            block->modelview = modelview;
            block->MVP = MVP;
            block->normalmatrix = normalMatrix;
            block->shadowMVP = shadowMVP;
            
            glUnmapBuffer(GL_UNIFORM_BUFFER);
            
            glUniform1ui(matParams.u_rendermode, rendermode);
            
            temple.mesh->render();
            
            glUseProgram(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    }
    
    virtual void onMouseButton(const int& button, const int& action)
    {
        int x, y;
        getMousePosition(x, y);
        arcball->onMouseButton(button, action, x, y);
    }
    
    virtual void onMouseMove(const int& x, const int& y)
    {
        arcball->onMouseMove(x, y);
    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case '1':
                    rendermode = RENDER_FULL;
                    break;
                case '2':
                    rendermode = RENDER_SHADOWMAP;
                    break;
                case '3':
                    rendermode = RENDER_NOSHADOWS;
                    break;
                case '4':
                    rendermode = RENDER_SHADOWSONLY;
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
        
        arcball->onResize(width, height);
    }

    virtual void shutdown()
    {
        delete temple.mesh;
        delete arcball;
        
        glDeleteTextures(1, &depthPass.depthbuffer);
        glDeleteFramebuffers(1, &depthPass.framebuffer);
        glDeleteProgram(matParams.progID);
        glDeleteProgram(depthPass.progID);
        glDeleteProgram(depthViz.progID);
        glDeleteBuffers(1, &transform_uniforms_buffer);
    }
};

DECLARE_MAIN(shadowTemples)
