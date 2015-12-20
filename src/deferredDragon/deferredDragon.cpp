#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <iostream>

using namespace glpg;
using namespace glpg::math;

class deferredDragon : public App
{
    enum
    {
        MAX_DISPLAY_WIDTH   = 2048,
        MAX_DISPLAY_HEIGHT  = 2048,
        MAX_NUM_LIGHTS      = 64
    };
    
    GLfloat clearColor[4];
    GLfloat clearDepth;
    
    GLuint gbuffer;
    GLuint gbufferTex[3];
    
    GLuint fullscreenQuadVao;

    GLuint lightProg;
    GLuint renderProg;
    GLuint vizProg;

    struct transforms_uniform_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };

    struct lights_uniform_block
    {
        math::vec4 light_pos; //vec4
        math::vec3 light_intensity; //vec3
        math::vec3 attenuation; //vec3 => Constant, Linear, Exp
    };
    
    GLuint  transforms_uniform_buffer;
    GLuint  lights_uniform_buffer;
    GLint   num_lights;

    struct
    {
        GLint material_id; //uint
        GLint diffuse_color; //vec3
        GLint specular_power; //float
    } matParams;
    
    GLint gbuffer_sampler_uniform[2];
    
    GLint viz_mode_uniform;
    
    GLint viz_sampler_uniform[2];
    
    enum
    {
        VIZ_OFF,
        VIZ_NORMALS,
        VIZ_POS,
        VIZ_DIFFUSE,
        VIZ_META
    } viz_mode;

    IMeshRenderer* frogMesh;

    float aspect;
    math::mat4 projection;

    virtual void settings()
    {
        mTitle = "Deferred Dragon";
        mSamples = 0;
    }

    virtual void setup()
    {
        
        // Set the clear color
        clearColor[0] = 0.5f; clearColor[1] = 0.5f; clearColor[2] = 0.5f; clearColor[3] = 0.0f;
        clearDepth = 1.0f;
        
        // BEGIN - Setup G-buffer
        glGenFramebuffers(1, &gbuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
        
        glGenTextures(3, gbufferTex);
        
        glBindTexture(GL_TEXTURE_2D, gbufferTex[0]);
        // 3 x 16-bit floating points for albedo color
        // 3 x 16-bit floating points for normals
        // 1 x 32-bit material ID (we really don't need that many)
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        // this info is exactly one per screen pixel. We don't want any interpolation
        // especially for data like normals or material IDs
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glBindTexture(GL_TEXTURE_2D, gbufferTex[1]);
        // 3 x world space coordiantes (view space?)
        // 1 x specular power (really, should be stored as lg(specular power)
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glBindTexture(GL_TEXTURE_2D, gbufferTex[2]);
        // full floating point depth buffer
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, MAX_DISPLAY_WIDTH, MAX_DISPLAY_HEIGHT);
        
        // Set the 3 textures as the attachment targets for the framebuffer to resolve to
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbufferTex[0], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbufferTex[1], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbufferTex[2], 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // END - Setup G-buffer
        
        // We need a full screen quad for final rendering
        // Not generating vertx buffer, vertices will be generated in the shader
        glGenVertexArrays(1, &fullscreenQuadVao);
        glBindVertexArray(fullscreenQuadVao);
        
        // Load the meshes for the scene
        frogMesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/Frog.obj");
        assert(frogMesh != NULL);
        
        // BEGIN - Setup programs for deferred shading
        
        //1. g-buffer filler
        GLuint shaders[2];
        
        shaders[0] = shader::fromFile("../shaders/deferred/render.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/deferred/render.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        renderProg = program::linkShaders(shaders, 2, true, true);
        assert(renderProg != 0);
        matParams.material_id = glGetUniformLocation(renderProg, "materialID");
        matParams.diffuse_color = glGetUniformLocation(renderProg, "matParams.diffuse_color");
        matParams.specular_power = glGetUniformLocation(renderProg, "matParams.specular_power");
        
        glGenBuffers(1, &transforms_uniform_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transforms_uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transforms_uniform_block), NULL, GL_DYNAMIC_DRAW);
        
        //2. ligthing model
        shaders[0] = shader::fromFile("../shaders/deferred/lighting.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/deferred/lighting.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        lightProg = program::linkShaders(shaders, 2, true, true);
        assert(lightProg != 0);
        num_lights = glGetUniformLocation(lightProg, "num_lights");
        gbuffer_sampler_uniform[0] = glGetUniformLocation(lightProg, "gbuffer_tex0");
        gbuffer_sampler_uniform[1] = glGetUniformLocation(lightProg, "gbuffer_tex1");
        
        glGenBuffers(1, &lights_uniform_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, lights_uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, MAX_NUM_LIGHTS * sizeof(lights_uniform_block), NULL, GL_DYNAMIC_DRAW);
        
        //3. viz helper
        shaders[0] = shader::fromFile("../shaders/deferred/lighting.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/deferred/viz.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        vizProg = program::linkShaders(shaders, 2, true, true);
        assert(vizProg != 0);
        viz_mode_uniform = glGetUniformLocation(vizProg, "viz_mode");
        viz_sampler_uniform[0] = glGetUniformLocation(vizProg, "gbuffer_tex0");
        viz_sampler_uniform[1] = glGetUniformLocation(vizProg, "gbuffer_tex1");
        
        // Set uniforms for shaders
        
        glUseProgram(renderProg);
        glUniform1ui(matParams.material_id, 1);
        glUniform3fv(matParams.diffuse_color, 1, math::vec3(0.072, 0.346, 0.072));
        glUniform1f(matParams.specular_power, 20.0f);

        glUseProgram(lightProg);
        glUniform1i(num_lights, 1);
        
        
        
        glUniform1i(gbuffer_sampler_uniform[0], 0);
        glUniform1i(gbuffer_sampler_uniform[1], 1);
        
        glUseProgram(vizProg);
        viz_mode = VIZ_OFF;
        glUniform1i(viz_mode_uniform, viz_mode);
        glUniform1i(viz_sampler_uniform[0], 0);
        glUniform1i(viz_sampler_uniform[1], 1);
        glUseProgram(0);
        
        
        // END - Setup programs for deferred shading

        

        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        //GLint error = glGetError();
        
        static const GLuint uint_zeros[] = { 0, 0, 0, 0 };
        static const GLfloat float_zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat float_ones[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        
        // BEGIN - update
        math::mat4 model = math::translate(0.0f, -0.4f, -2.0f)
            * math::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
            * math::scale(0.003f);
        
        math::mat4 MVP = projection * model;
        
        math::mat4 normalMatrix = model.inverse().transpose();
        // END - update
        
        // We start by binding the g framebuffer and setting the draw buffers
        glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
        glDrawBuffers(2, draw_buffers);
        
        // clear all render textures
        glClearBufferuiv(GL_COLOR, 0, uint_zeros);
        glClearBufferfv(GL_COLOR, 1, float_zeros);
        glClearBufferfv(GL_DEPTH, 0, float_ones);
        
        // fill the buffer for the transforms uniform block
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transforms_uniform_buffer);
        transforms_uniform_block* block
            = (transforms_uniform_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                          0,
                                                          sizeof(transforms_uniform_block),
                                                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        
        block->modelview = model;
        block->MVP = MVP;
        block->normalmatrix = normalMatrix;
        
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        // Time to render the scene info for deferred rendering

        glUseProgram(renderProg);
        
        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        frogMesh->render();
        
        // Done rendering to a texture!
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Now let's render on screen - lighting pass!
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);
        glDrawBuffer(GL_BACK); //we set back buffer again
        
        // Bind the freshly filled render textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbufferTex[0]);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbufferTex[1]);
        
        if(viz_mode == VIZ_OFF)
        {
            glUseProgram(lightProg);
            
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, lights_uniform_buffer);
            lights_uniform_block* lights
            = reinterpret_cast<lights_uniform_block*>(glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                       0,
                                                                       MAX_NUM_LIGHTS * sizeof(lights_uniform_block),
                                                                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
            
            lights[0].light_pos = math::vec4(0.25, -0.25, -0.25, 0.0);
            lights[0].light_intensity = math::vec3(1.0);
            
            glUnmapBuffer(GL_UNIFORM_BUFFER);
        }
        else
        {
            glUseProgram(vizProg);
            glUniform1i(viz_mode_uniform, viz_mode);
        }

        glDisable(GL_DEPTH_TEST); //we don't use depth test to render a quad on screen
        
        // Bind quad vao and draw quad
        glBindVertexArray(fullscreenQuadVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw a strip of four vertices
        
        // clean up
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case '0':
                    viz_mode = VIZ_OFF;
                    break;
                case '1':
                    viz_mode = VIZ_NORMALS;
                    break;
                case '2':
                    viz_mode = VIZ_POS;
                    break;
                case '3':
                    viz_mode = VIZ_DIFFUSE;
                    break;
                case '4':
                    viz_mode = VIZ_META;
                    break;
            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = math::perspective(50.0f, aspect, 0.1f, 1000.0f);
    }

    virtual void shutdown()
    {
        delete frogMesh;

        glDeleteTextures(3, &gbufferTex[0]);
        glDeleteFramebuffers(1, &gbuffer);
        glDeleteProgram(renderProg);
        glDeleteProgram(vizProg);
        glDeleteProgram(lightProg);
        
    }
};

DECLARE_MAIN(deferredDragon)
