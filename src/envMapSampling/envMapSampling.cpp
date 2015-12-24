#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <iostream>
#include <SOIL.h>

using namespace glpg;
using namespace glpg::math;

class envMapSampling : public App
{
    typedef enum EnvMapType
    {
        ENV_MAP_CUBE = 0,
        ENV_MAP_SPHERICAL,
        ENV_MAP_EQUI
    } EnvMapType;
    
    GLfloat clearColor[4];
    GLfloat clearDepth;

    typedef struct EnvMapMat
    {
        GLuint progID;
        GLint envmap_uniform;
        
        static void LoadMat(const GLuint& vertex_shader, const char* fragment_shader_name, EnvMapMat& mat)
        {
            GLuint shaders[2];
            shaders[0] = vertex_shader;
            shaders[1] = shader::fromFile("../shaders/envmapsphere/envmapsphere.frag", GL_FRAGMENT_SHADER, true);
            assert(shaders[1] != 0);
            mat.progID = program::linkShaders(shaders, 2, true, true);
            assert(mat.progID != 0);
            mat.envmap_uniform = glGetUniformLocation(mat.progID, "tex_envmap");
            glUseProgram(mat.progID);
            glUniform1i(mat.envmap_uniform, 0);
            glUseProgram(0);
        }
        
    } EnvMapMat;
    
    EnvMapMat mEnvMapMaterials[3];
    
    typedef struct EnvMapTex
    {
        GLuint texID;
        EnvMapType type;
        
        static void LoadTex(EnvMapTex& tex, EnvMapType inType, const char* name)
        {
            tex.type = inType;
            tex.texID = SOIL_load_OGL_texture(name, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
            assert(0 != tex.texID);
        }
        
    } EnvMapTex;
    
    EnvMapTex mEnvMaps[7];

    struct transform_uniforms_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };

    GLuint transform_uniforms_buffer;

    IMeshRenderer* teapotMesh;
    
    EnvMapTex* current_tex;
    EnvMapMat* current_mat;

    float aspect;
    math::mat4 projection;
    
    void setEnvMap(EnvMapTex& tex)
    {
        current_mat = &mEnvMapMaterials[tex.type];
        current_tex = &tex;
    }

    virtual void settings()
    {
        mTitle = "EnvMap Teapot";
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
        GLuint vert_shader = shader::fromFile("../shaders/envmapsampling/pass.vert", GL_VERTEX_SHADER, true);
        assert(vert_shader != 0);
        
        EnvMapMat::LoadMat(vert_shader, "../shaders/envmapsampling/envmapsphere.frag", mEnvMapMaterials[ENV_MAP_SPHERICAL]);
        EnvMapMat::LoadMat(vert_shader, "../shaders/envmapsampling/envmapequi.frag", mEnvMapMaterials[ENV_MAP_EQUI]);
        EnvMapMat::LoadMat(vert_shader, "../shaders/envmapsampling/envmapcube.frag", mEnvMapMaterials[ENV_MAP_CUBE]);

        glGenBuffers(1, &transform_uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transform_uniforms_block), NULL, GL_DYNAMIC_DRAW);

        teapotMesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/teapot.obj");

        assert(teapotMesh != NULL);
        
        EnvMapTex::LoadTex(mEnvMaps[0], ENV_MAP_SPHERICAL, "../textures/envmaps/uffizi.png");
        EnvMapTex::LoadTex(mEnvMaps[1], ENV_MAP_SPHERICAL, "../textures/envmaps/sea.png");
        EnvMapTex::LoadTex(mEnvMaps[2], ENV_MAP_SPHERICAL, "../textures/envmaps/pub.png");
        EnvMapTex::LoadTex(mEnvMaps[3], ENV_MAP_SPHERICAL, "../textures/envmaps/crossroad.png");
        EnvMapTex::LoadTex(mEnvMaps[4], ENV_MAP_EQUI, "../textures/envmaps/snow.png");
        EnvMapTex::LoadTex(mEnvMaps[5], ENV_MAP_EQUI, "../textures/envmaps/mountains.png");
        EnvMapTex::LoadTex(mEnvMaps[6], ENV_MAP_SPHERICAL, "../textures/envmaps/church.png");
        
        this->setEnvMap(mEnvMaps[0]);

        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);

        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);

        glUseProgram(current_mat->progID);

        // set the uniforms buffer

        math::mat4 model = math::translate(0.0f, -0.5f, -2.0f)
                            * math::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
                            * math::scale(0.01f);
        math::mat4 MVP = projection * model;

        math::mat4 normalMatrix = model.inverse().transpose();

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_uniforms_buffer);
        transform_uniforms_block* block = (transform_uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                  0,
                                                                  sizeof(transform_uniforms_block),
                                                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        block->modelview = model;
        block->MVP = MVP;
        block->normalmatrix = normalMatrix;

        glUnmapBuffer(GL_UNIFORM_BUFFER);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, current_tex->texID);

        teapotMesh->render();

        //GLint error = glGetError();

    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case '1':
                    setEnvMap(mEnvMaps[0]);
                    break;
                case '2':
                    setEnvMap(mEnvMaps[1]);
                    break;
                case '3':
                    setEnvMap(mEnvMaps[2]);
                    break;
                case '4':
                    setEnvMap(mEnvMaps[3]);
                    break;
                case '5':
                    setEnvMap(mEnvMaps[4]);
                    break;
                case '6':
                    setEnvMap(mEnvMaps[5]);
                    break;
                case '7':
                    setEnvMap(mEnvMaps[6]);
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
        delete teapotMesh;

        glDeleteProgram(mEnvMapMaterials[0].progID);
        glDeleteProgram(mEnvMapMaterials[1].progID);
        glDeleteProgram(mEnvMapMaterials[2].progID);
        glDeleteBuffers(1, &transform_uniforms_buffer);
    }
};

DECLARE_MAIN(envMapSampling)
