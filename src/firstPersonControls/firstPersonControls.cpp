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

class firstPersonControls : public App
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
    } matParams;
    
    struct
    {
        IMeshRenderer* mesh;
        glm::mat4 transform;
    } temple;

    float aspect;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 root;

    virtual void settings()
    {
        mTitle = "First Person Controls";
    }

    virtual void setup()
    {
        clearColor[0] = 0.5f; clearColor[1] = 0.5f; clearColor[2] = 0.5f; clearColor[3] = 0.0f;
        clearDepth = 1.0f;
        
        view = glm::lookAt(glm::vec3(-2.0f, 2, 0.0f), glm::vec3(-1.0,2,0), glm::vec3(0, 1.0, 0));
        lightParams.light_pos = glm::vec4(glm::normalize(glm::vec3(0.5, -0.5, 0)), 0.0);

        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/phong/phong.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/phong/phong.frag", GL_FRAGMENT_SHADER, true);
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

        glUseProgram(matParams.progID);
        glUniform3fv(matParams.u_ambient_color, 1, math::vec3(0.0));
        glUniform3fv(matParams.u_diffuse_color, 1, math::vec3(1.0, 1.0, 1.0));
        glUniform3fv(matParams.u_specular_color, 1, math::vec3(1.0, 1.0, 1.0));
        glUniform1f(matParams.u_specular_power, 128.0f);
        glm::vec4 lightEyeSpace = view * lightParams.light_pos;
        lightEyeSpace = glm::vec4(lightEyeSpace.x, lightEyeSpace.y, lightEyeSpace.z, 0.0);
        glUniform4fv(lightParams.u_light_pos, 1, glm::value_ptr(lightEyeSpace));
        glUniform3fv(lightParams.u_light_intensity, 1, math::vec3(1.0));
        glUseProgram(0);
        
        glGenBuffers(1, &transform_uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transform_uniforms_block), NULL, GL_DYNAMIC_DRAW);
        
        temple.mesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/dabrovic-sponza/sponza.obj");
        assert(temple.mesh != NULL);
        temple.transform = glm::scale(glm::mat4(), glm::vec3(1.0f));
        
        root = glm::mat4();

        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        onResize(mWindowWidth, mWindowHeight);
    }
    
    virtual void render(double currentTime)
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
        
        glUseProgram(matParams.progID);
        
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_uniforms_buffer);
        transform_uniforms_block* block = (transform_uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                                      0,
                                                                                      sizeof(transform_uniforms_block),
                                                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        
        block->modelview = modelview;
        block->MVP = MVP;
        block->normalmatrix = normalMatrix;
        
        glUnmapBuffer(GL_UNIFORM_BUFFER);
        
        temple.mesh->render();
        
        glUseProgram(0);
        
    }
    
    virtual void onMouseButton(const int& button, const int& action)
    {
        int x, y;
        getMousePosition(x, y);
    }
    
    virtual void onMouseMove(const int& x, const int& y)
    {
    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
//            switch (key)
//            {
//            }
        }
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
        

    }

    virtual void shutdown()
    {
        delete temple.mesh;
        glDeleteProgram(matParams.progID);
        glDeleteBuffers(1, &transform_uniforms_buffer);
    }
};

DECLARE_MAIN(firstPersonControls)
