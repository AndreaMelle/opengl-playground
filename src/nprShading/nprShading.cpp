#include <glpgApp.h>
#include <glpgMath.h>
#include <glpgShader.h>
#include <glpgBaseMeshRenderer.h>
#include <glpgBaseMeshLoader.h>
#include <glpgCompositeIndexdedMeshRenderer.h>
#include <glpgAssImpLoader.h>
#include <glpgPrimitives.h>
#include <iostream>
#include <SOIL.h>
#include <AntTweakBar.h>

using namespace glpg;
using namespace glpg::math;

void TW_CALL SetModelTypeCB(const void *value, void *clientData);
void TW_CALL GetModelTypeCB(void *value, void *clientData);

class nprShading : public App
{
public:
    typedef enum ModelType
    {
        MODEL_SPHERE = 1,
        MODEL_OLDMAN = 2,
        MODEL_TORUSKNOT = 3,
    } ModelType;
    
    void setModelType(ModelType type)
    {
        switch (type) {
            case MODEL_OLDMAN:
                current_object = &oldmanObject;
                break;
            case MODEL_SPHERE:
                current_object = &sphereObject;
                break;
            case MODEL_TORUSKNOT:
                current_object = &knotTorusObject;
                break;
            default:
                break;
        }
        mModelType = type;
    }
    
    ModelType getModelType() const { return mModelType; }
    
private:
    
    // clear values
    GLfloat clearColor[4];
    GLfloat clearDepth;

    // shaders
    GLuint nprProgramID;

    struct
    {
        GLint light_pos; //vec4
        GLint light_intensity; //vec3
        GLint attenuation; //vec3 => Constant, Linear, Exp
    } lightParams;

    struct
    {
        GLint material_mode;
        GLint gradientwrap_lut;
        GLint diffuse_tex_uniform;
        //GLint ambient_color; //vec3
        GLint diffuse_color; //vec3
        //GLint specular_color; //vec3
        //GLint specular_power; //float
        GLint rim_power;
    } matParams;
    
    float mRimPower;
    float mColorDiffuse[3];
    
    GLuint gradientwrap_tex;
    GLuint diffuse_tex;
    
    typedef struct SceneObject
    {
        IMeshRenderer* meshRenderer;
        math::mat4 modelTransform;
    } SceneObject;
    
    SceneObject* current_object;
    
    SceneObject sphereObject;
    SceneObject oldmanObject;
    SceneObject knotTorusObject;

    // camera transforms
    float aspect;
    math::mat4 view;
    math::mat4 projection;
    math::mat4 normalMatrix;
    
    struct transform_uniforms_block
    {
        math::mat4 modelview;
        math::mat4 MVP;
        math::mat4 normalmatrix;
    };
    
    GLuint transform_uniforms_buffer;
    
    // light transform
    math::vec3 light_dir;
    
    // mode
    typedef enum ShaderMode
    {
        RENDER_GRADIENTWRAP = 1,
        RENDER_LAMBERT = 2,
        RENDER_RIM = 3,
        RENDER_CINEMATIC_DIFFUSE = 4,
    } ShaderMode;
    
    ShaderMode mShaderMode;
    
    ModelType mModelType;
    
    TwBar *bar;
    
    
    virtual void settings()
    {
        mTitle = "NPR";
        mWindowWidth = 800;
        mWindowHeight = 500;
        mSamples = 4;
    }

    virtual void setup()
    {
        clearColor[0] = 0.2f; clearColor[1] = 0.2f; clearColor[2] = 0.2f; clearColor[3] = 0.0f;
        clearDepth = 1.0f;

        GLuint shaders[2];
        shaders[0] = shader::fromFile("../shaders/npr/npr.vert", GL_VERTEX_SHADER, true);
        shaders[1] = shader::fromFile("../shaders/npr/npr.frag", GL_FRAGMENT_SHADER, true);
        assert(shaders[0] != 0 && shaders[1] != 0);
        nprProgramID = program::linkShaders(shaders, 2, true, true);
        assert(nprProgramID != 0);
        lightParams.light_pos = glGetUniformLocation(nprProgramID, "light_pos");
        lightParams.light_intensity = glGetUniformLocation(nprProgramID, "lightParams.light_intensity");
        lightParams.attenuation = glGetUniformLocation(nprProgramID, "lightParams.attenuation");

        //matParams.ambient_color = glGetUniformLocation(nprProgramID, "matParams.ambient_color");
        matParams.diffuse_color = glGetUniformLocation(nprProgramID, "matParams.diffuse_color");
        matParams.gradientwrap_lut = glGetUniformLocation(nprProgramID, "gradientwrap_tex");
        matParams.material_mode = glGetUniformLocation(nprProgramID, "material_mode");
        matParams.rim_power = glGetUniformLocation(nprProgramID, "matParams.rim_power");
        matParams.diffuse_tex_uniform = glGetUniformLocation(nprProgramID, "diffuse_tex");
        //matParams.specular_color = glGetUniformLocation(nprProgramID, "matParams.specular_color");
        //matParams.specular_power = glGetUniformLocation(nprProgramID, "matParams.specular_power");

        mShaderMode = RENDER_CINEMATIC_DIFFUSE;
        mColorDiffuse[0] = 1.0f;//237.0f / 255.0f;
        mColorDiffuse[1] = 1.0f;//150.0f / 255.0f;
        mColorDiffuse[2] = 1.0f;//133.0f / 255.0f;
        //math::vec3 ambientColor(0.1f);
        //math::vec3 specularColor(1.0f);
        //float specularPower = 128.0f;
        mRimPower = 3.0f;
        
        light_dir = math::vec3(-0.84, -0.36, -1.19f);
        math::vec3 lightIntensity(1.0f);

        glUseProgram(nprProgramID);

        //glUniform3fv(matParams.ambient_color, 1, ambientColor);
        glUniform3fv(matParams.diffuse_color, 1, mColorDiffuse);
        glUniform1i(matParams.gradientwrap_lut, 0);
        glUniform1i(matParams.diffuse_tex_uniform, 1);
        glUniform1i(matParams.material_mode, mShaderMode);
        //glUniform3fv(matParams.specular_color, 1, specularColor);
        //glUniform1f(matParams.specular_power, specularPower);

        glUniform4fv(lightParams.light_pos, 1, math::vec4(light_dir, 0.0));
        glUniform3fv(lightParams.light_intensity, 1, lightIntensity);
        glUseProgram(0);

        glGenBuffers(1, &transform_uniforms_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_uniforms_buffer);
        // Defined dynamic because we animate
        glBufferData(GL_UNIFORM_BUFFER, sizeof(transform_uniforms_block), NULL, GL_DYNAMIC_DRAW);

        sphereObject.meshRenderer = UnitSphere::CreateMesh();

        assert(sphereObject.meshRenderer != NULL);
        
        sphereObject.modelTransform = math::translate(0.0f, 0.0f, -5.0f) * math::scale(1.0f);
        
        oldmanObject.meshRenderer = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/oldman/muro.obj");
        assert(oldmanObject.meshRenderer != NULL);
        

        oldmanObject.modelTransform = math::translate(0.0f, -2.2f, -5.0f) * math::scale(0.025f);
        
        knotTorusObject.meshRenderer = IMeshLoader<AssImpLoader, CompositeIndexedMeshRenderer>::Create("../models/torusknot.obj");
        assert(knotTorusObject.meshRenderer != NULL);
        
        knotTorusObject.modelTransform = math::translate(0.0f, 0.0f, -5.0f) * math::scale(0.7f);
        
        mModelType = MODEL_TORUSKNOT;
        current_object = &knotTorusObject;
        
        view = math::mat4::identity();
        
        gradientwrap_tex = SOIL_load_OGL_texture("../textures/LUTs/tythonDiffuseGradient.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
        assert(0 != gradientwrap_tex);
        
        diffuse_tex = SOIL_load_OGL_texture("../textures/oldMan/oldManBodyDiffuse.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
        assert(0 != diffuse_tex);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gradientwrap_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glEnable(GL_CULL_FACE); glFrontFace(GL_CCW);
        glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
        
        TwDefine(" GLOBAL fontscaling=2 ");
        TwInit(TW_OPENGL_CORE, NULL);
        
        
        
        bar = TwNewBar("NPRParameters");
        
        TwDefine(" NPRParameters size='500 500' color='128 128 128' ");

        TwAddVarRW(bar, "LightDir", TW_TYPE_DIR3F, (&light_dir[0]), " label='Light direction' opened=true ");
        
        {
            TwEnumVal shaderEV[4] = { {RENDER_GRADIENTWRAP, "Gradient Wrap"},
                {RENDER_LAMBERT, "Lambert"},
                {RENDER_RIM, "Rim"},
                {RENDER_CINEMATIC_DIFFUSE, "Cinematic Diffuse"} };
                
            TwType shaderType = TwDefineEnum("ShaderModeType", shaderEV, 4);
            TwAddVarRW(bar, "ShaderMode", shaderType, &mShaderMode, " keyIncr='<' keyDecr='>' help='Change shader mode.' ");
        }
        
        {
            TwEnumVal modelEV[3] = { {MODEL_SPHERE, "Sphere"}, {MODEL_OLDMAN, "Old Man"}, {MODEL_TORUSKNOT, "Torusk Knot"} };
            TwType modelType = TwDefineEnum("ModelType", modelEV, 3);
            TwAddVarCB(bar, "Model", modelType, SetModelTypeCB, GetModelTypeCB, this, " keyIncr='a' keyDecr='s' help='Change model.' ");
        }
        
        TwAddVarRW(bar, "Rim Power", TW_TYPE_FLOAT, &mRimPower, " group='Material' min=0 max=16 step=0.1 ");
        TwAddVarRW(bar, "Diffuse", TW_TYPE_COLOR3F, &mColorDiffuse, " group='Material' ");
        
        onResize(mWindowWidth, mWindowHeight);

    }

    virtual void render(double currentTime)
    {
        
        glViewport(0, 0, 2 * mWindowWidth, 2 * mWindowHeight);

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, &clearDepth);
        
        normalMatrix = view * current_object->modelTransform;
        normalMatrix = normalMatrix.inverse().transpose();
        
        glUseProgram(nprProgramID);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_uniforms_buffer);
        transform_uniforms_block* block = (transform_uniforms_block*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                                      0,
                                                                                      sizeof(transform_uniforms_block),
                                                                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        block->modelview = view * current_object->modelTransform;
        block->MVP = projection * view *  current_object->modelTransform;
        block->normalmatrix = normalMatrix;

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glUniform4fv(lightParams.light_pos, 1, math::vec4(light_dir, 0.0));
        glUniform1i(matParams.material_mode, mShaderMode);
        glUniform3fv(matParams.diffuse_color, 1, mColorDiffuse);
        glUniform1f(matParams.rim_power, mRimPower);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gradientwrap_tex);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, diffuse_tex);
        
        current_object->meshRenderer->render();

        glUseProgram(0);
        
        TwDraw();
        
    }

    virtual void onKey(const int& key, const int& action)
    {
        if(TwEventKeyGLFW(key, action) != 0) return;
        
//        if (action)
//        {
//            switch (key)
//            {
//            }
//        }
    }
    
    virtual void onChar(const unsigned int& c)
    {
        TwEventCharGLFW(c, GLFW_PRESS);
    }
    
    virtual void onMouseButton(const int& button, const int& action)
    {
        TwEventMouseButtonGLFW(button, action);
    }
    
    virtual void onMouseMove(const int& x, const int& y)
    {
        TwEventMousePosGLFW(x * 2, y * 2);
    }
    virtual void onMouseWheel(const int& pos)
    {
        TwEventMouseWheelGLFW(pos);
    }

    virtual void onResize(const int& width, const int& height)
    {
        aspect = (float)width / (float)height;
        projection = math::perspective(60.0f, aspect, 0.1f, 1000.0f);
        TwWindowSize(2 * mWindowWidth, 2 * mWindowHeight);
    }

    virtual void shutdown()
    {
        TwTerminate();
        
        delete sphereObject.meshRenderer;
        delete oldmanObject.meshRenderer;
        glDeleteProgram(nprProgramID);
        glDeleteBuffers(1, &transform_uniforms_buffer);
    }
};

void TW_CALL SetModelTypeCB(const void *value, void *clientData)
{
    nprShading* app = (nprShading*)clientData;
    nprShading::ModelType type = *(const nprShading::ModelType*)value;
    app->setModelType(type);
}

void TW_CALL GetModelTypeCB(void *value, void *clientData)
{
    nprShading* app = (nprShading*)clientData;
    *(int *)value = (int)app->getModelType();
}

DECLARE_MAIN(nprShading)
