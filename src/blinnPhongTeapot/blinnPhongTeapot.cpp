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

class blinnPhongTeapot : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;

    GLuint programID;

    struct uniforms_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };

    GLuint uniforms_buffer;

    struct
    {
        GLint light_pos; //vec4
        GLint light_intensity; //vec3
        GLint attenuation; //vec3 => Constant, Linear, Exp
    } lightParams;

    struct
    {
        GLint ambient_color; //vec3
        GLint diffuse_color; //vec3
        GLint specular_color; //vec3
        GLint specular_power; //float
    } matParams;

    GLuint normalVizProgramID;
    GLint normalVizMVPUniform;
    GLint normalLengthUniform;

    IMeshRenderer* teapotMesh;

    float aspect;
    math::mat4 projection;

    bool showDebugNormals;

    virtual void settings()
    {
        mTitle = "BlinnPhong Teapot";
    }

    virtual void setup()
    {
        showDebugNormals = false;

        // Set the clear color
        clearColor[0] = 0.5f;
        clearColor[1] = 0.5f;
        clearColor[2] = 0.5f;
        clearColor[3] = 0.0f;
        clearDepth = 1.0f;

        // Create and compile shader
        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/phong/phong.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/phong/phong.frag", GL_FRAGMENT_SHADER, true);

        if (shaders[0] == 0 || shaders[1] == 0)
        {
            std::cout << "Failed to load shaders" << std::endl;
            exit(-1);
        }

        programID = program::linkShaders(shaders, 2, true, true);
        if(programID == 0)
        {
          std::cout << "Failed to link program" << std::endl;
          exit(-1);
        }

        // Get the uniforms from the shader
        lightParams.light_pos = glGetUniformLocation(programID, "light_pos");
        lightParams.light_intensity = glGetUniformLocation(programID, "lightParams.light_intensity");
        lightParams.attenuation = glGetUniformLocation(programID, "lightParams.attenuation");

        matParams.ambient_color = glGetUniformLocation(programID, "matParams.ambient_color");
        matParams.diffuse_color = glGetUniformLocation(programID, "matParams.diffuse_color");
        matParams.specular_color = glGetUniformLocation(programID, "matParams.specular_color");
        matParams.specular_power = glGetUniformLocation(programID, "matParams.specular_power");

        glUseProgram(programID);

        glUniform3fv(matParams.ambient_color, 1, math::vec3(0.0));
        glUniform3fv(matParams.diffuse_color, 1, math::vec3(1.0, 0, 0));
        glUniform3fv(matParams.specular_color, 1, math::vec3(1.0, 1.0, 1.0));
        glUniform1f(matParams.specular_power, 128.0f);

        glUniform4fv(lightParams.light_pos, 1, math::vec4(0.25, -0.25, -0.25, 0.0));
        glUniform3fv(lightParams.light_intensity, 1, math::vec3(1.0));

        glUseProgram(0);

        // Create and compile the normal viz helper shader
        GLuint normalShaders[3];
        normalShaders[0] = shader::fromFile("../shaders/normalViz/normal.vert", GL_VERTEX_SHADER, true);
        normalShaders[1] = shader::fromFile("../shaders/normalViz/normal.geom", GL_GEOMETRY_SHADER, true);
        normalShaders[2] = shader::fromFile("../shaders/normalViz/normal.frag", GL_FRAGMENT_SHADER, true);

        if (normalShaders[0] == 0 || normalShaders[1] == 0 || normalShaders[2] == 0)
        {
            std::cout << "Failed to load normal viz shaders" << std::endl;
            exit(-1);
        }

        normalVizProgramID = program::linkShaders(normalShaders, 3, true, true);
        if(normalVizProgramID == 0)
        {
            std::cout << "Failed to link normal viz program" << std::endl;
            exit(-1);
        }

        normalVizMVPUniform = glGetUniformLocation(normalVizProgramID, "MVP");
        normalLengthUniform = glGetUniformLocation(normalVizProgramID, "normal_length");

        // Generate the uniform buffer
        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
        // Defined dynamic because we animate
        glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

        teapotMesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/teapot.obj");

        if(teapotMesh == NULL)
        {
            std::cout << "Failed to load model." << std::endl;
            exit(-1);
        }


        //cull front faces
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        //standard depth test
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);

        glUseProgram(programID);

        // set the uniforms buffer

        math::mat4 model = math::translate(0.0f, -0.5f, -2.0f)
                            * math::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f)
                            * math::scale(0.01f);
        math::mat4 MVP = projection * model;

        math::mat4 normalMatrix = model.inverse().transpose();

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniforms_buffer);
        uniforms_block* block = (uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                  0,
                                                                  sizeof(uniforms_block),
                                                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        block->modelview = model;
        block->MVP = MVP;
        block->normalmatrix = normalMatrix;

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        teapotMesh->render();

        if(showDebugNormals)
        {
            glUseProgram(normalVizProgramID);
            glUniformMatrix4fv(normalVizMVPUniform, 1, GL_FALSE, MVP);
            glUniform1f(normalLengthUniform, 2.0f);
            teapotMesh->render();
        }

        //GLint error = glGetError();

    }

    virtual void onKey(const int& key, const int& action)
    {
        if (action)
        {
            switch (key)
            {
                case 'N':
                    showDebugNormals = !showDebugNormals;
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

        glDeleteProgram(programID);
        glDeleteBuffers(1, &uniforms_buffer);
    }
};

DECLARE_MAIN(blinnPhongTeapot)
