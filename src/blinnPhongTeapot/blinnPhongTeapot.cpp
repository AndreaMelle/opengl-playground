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

using namespace glpg;
using namespace glpg::math;

class blinnPhongTeapot : public App
{
    GLfloat clearColor[4];
    GLfloat clearDepth;

    GLuint programID;

    struct uniforms_block
    {
        glm::mat4 modelview;
        glm::mat4 MVP;
        glm::mat4 normalmatrix;
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
    glm::mat4 projection;

    bool showDebugNormals;
    
    struct
    {
        GLuint progID;
        GLint color_uniform;
        math::vec3 color;
        GLint transform_uniform;
    } unlitMat;
    
    struct
    {
        IMeshRenderer* mesh;
        glm::mat4 transform;
    } grid;
    
    struct
    {
        IMeshRenderer* mesh;
        glm::mat4 transform;
        
    } sphere;
    
    Arcball* arcball;

    virtual void settings()
    {
        mTitle = "BlinnPhong Teapot";
    }
    
    glm::mat4 m;

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
        
        // Unlit shader
        GLuint unlitShaders[2];
        unlitShaders[0] = shader::fromFile("../shaders/unlit/pass.vert", GL_VERTEX_SHADER, true);
        unlitShaders[1] = shader::fromFile("../shaders/unlit/unlit_color.frag", GL_FRAGMENT_SHADER, true);
        assert(unlitShaders[0] != 0 && unlitShaders[1] != 0);
        unlitMat.progID = program::linkShaders(unlitShaders, 2, true, true);
        assert(unlitMat.progID != 0);
        unlitMat.color_uniform = glGetUniformLocation(unlitMat.progID, "color");
        unlitMat.transform_uniform = glGetUniformLocation(unlitMat.progID, "MVP");
        unlitMat.color = math::vec3(1.0f, 1.0f, 1.0f);
        glUseProgram(unlitMat.progID);
        glUniform3fv(unlitMat.color_uniform, 1, unlitMat.color);
        glUseProgram(0);
        

        // Generate the uniform buffer
        glGenBuffers(1, &uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);
        
        teapotMesh = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/teapot.obj");
        assert(teapotMesh != NULL);

        grid.mesh = glpg::UnitGrid::Create(5.0f, 5.0f, 10, 10);
        assert(grid.mesh != NULL);
        grid.transform = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.5f, -4.0f));
        
        //sphere.mesh = UnitSphere::CreateMesh();
        //assert(sphere.mesh != NULL);
        
        //sphere.transform= glm::translate(0.0f, 0.0f, -5.0f) * math::scale(1.0f);

        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        arcball = new Arcball(mWindowWidth, mWindowHeight);

        onResize(mWindowWidth, mWindowHeight);
        
        m = glm::mat4();

    }
    
    
    
    virtual void render(double currentTime)
    {
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);
        
        arcball->update(glm::mat4(), m);
        
        glm::mat4 glmMVP = projection * grid.transform * m;
        
        glUseProgram(unlitMat.progID);

        glUniformMatrix4fv(unlitMat.transform_uniform, 1, GL_FALSE, glm::value_ptr(glmMVP));
        
        grid.mesh->render();
        
        glUseProgram(0);
        
        glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(0.0f, -0.5f, -4.0f))
        * m * glm::rotate(glm::mat4(), glm::radians((float)currentTime * 45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(), glm::vec3(0.01f, 0.01f, 0.01f));
        
        glm::mat4 MVP = projection * model;
        
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

        glUseProgram(programID);

        // set the uniforms buffer
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
            glUniformMatrix4fv(normalVizMVPUniform, 1, GL_FALSE, glm::value_ptr(MVP));
            glUniform1f(normalLengthUniform, 2.0f);
            teapotMesh->render();
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
                case 'N':
                    showDebugNormals = !showDebugNormals;
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
        delete teapotMesh;
        delete arcball;

        glDeleteProgram(programID);
        glDeleteBuffers(1, &uniforms_buffer);
    }
};

DECLARE_MAIN(blinnPhongTeapot)
