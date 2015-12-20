#ifndef __GLPG_PRIMITIVES_H__
#define __GLPG_PRIMITIVES_H__

#include <glpgMath.h>
#include <vector>

namespace glpg
{
    class UnitSphere
    {
    public:
        static UnitSphere& Get()
        {
            static UnitSphere instance;
            return instance;
        }
        
        static inline int DrawMode() { return GL_TRIANGLE_STRIP; }
        static inline int VertexCount() { return Get().mVertexCount; }
        static inline const std::vector<math::vec3>& Vertices() { return Get().mVertices; }
        static inline const std::vector<math::vec2>& UVs() { return Get().mUVs; }
        static inline const std::vector<math::vec3>& Normals() { return Get().mNormals; }
        
    private:
        UnitSphere(const UnitSphere&);
        void operator=(UnitSphere const&);
        
        UnitSphere() { create(); };
        
        std::vector<math::vec3> mVertices;
        std::vector<math::vec2> mUVs;
        std::vector<math::vec3> mNormals;
        
        unsigned int mVertexCount;
        
        void create()
        {
            double sphereRadius = 1.0;
            int spherePrecision = 64;
            
            double theta1 = 0.0;
            double theta2 = 0.0;
            double theta3 = 0.0;
            
            double px, py, pz, pu, pv, nx, ny, nz;
            
            mVertices.clear();
            mUVs.clear();
            mNormals.clear();
            mVertexCount = 0;
            
            for( int i = 0; i < spherePrecision / 2; ++i )
            {
                theta1 = i * 2.0 * M_PI / spherePrecision - M_PI * 0.5;
                theta2 = ((double)i + 1.0) * 2.0 * M_PI / spherePrecision - M_PI * 0.5;
                
                for( int j = 0; j <= spherePrecision; ++j )
                {
                    theta3 = (double)j * 2.0 * M_PI / spherePrecision;
                    
                    nx = -cos(theta2) * cos(theta3);
                    ny = -sin(theta2);
                    nz = -cos(theta2) * sin(theta3);
                    
                    px = sphereRadius * nx;
                    py = sphereRadius * ny;
                    pz = sphereRadius * nz;
                    
                    pu  = ((double)j / (double)spherePrecision);
                    pv  = 2.0 * ((double)i + 1.0) / (double)spherePrecision;
                    
                    mVertices.push_back(math::vec3(px, py, pz));
                    mUVs.push_back(math::vec2(pu, pv));
                    mNormals.push_back(math::vec3(nx, ny, nz));
                    
                    nx = -cos(theta1) * cos(theta3);
                    ny = -sin(theta1);
                    nz = -cos(theta1) * sin(theta3);
                    
                    px = sphereRadius * nx;
                    py = sphereRadius * ny;
                    pz = sphereRadius * nz;
                    
                    pu  = ((double)j/(double)spherePrecision);
                    pv  = 2.0 * (double)i / (double)spherePrecision;
                    
                    mVertices.push_back(math::vec3(px, py, pz));
                    mUVs.push_back(math::vec2(pu, pv));
                    mNormals.push_back(math::vec3(nx, ny, nz));
                    
                    mVertexCount +=2;
                }
            }
        }
        
    };
    
  class UnitCube
  {
  public:
      static UnitCube& Get()
      {
          static UnitCube instance;
          return instance;
      }
      
      static inline int VertexCount() { return gVertexCount; }
      
      static inline const std::vector<math::vec3>& Vertices() { return Get().mVertices; }
      static inline const std::vector<math::vec2>& UVs() { return Get().mUVs; }
      static inline const std::vector<math::vec3>& Normals() { return Get().mNormals; }
      
  private:
      UnitCube() { create(); };
      
      UnitCube(const UnitCube&);
      void operator=(UnitCube const&);
      
      std::vector<math::vec3> mVertices;
      std::vector<math::vec2> mUVs;
      std::vector<math::vec3> mNormals;
      
      static const unsigned int gVertexCount = 36;
      
      void create()
      {
          
          GLfloat v[] = {
              -1.0f,  1.0f, -1.0f,
              1.0f, -1.0f, -1.0f,
              -1.0f, -1.0f, -1.0f,
              
              1.0f, -1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
              1.0f,  1.0f, -1.0f,
              
              1.0f, -1.0f, -1.0f,
              1.0f,  1.0f, -1.0f,
              1.0f, -1.0f,  1.0f,
              
              1.0f, -1.0f,  1.0f,
              1.0f,  1.0f, -1.0f,
              1.0f,  1.0f,  1.0f,
              
              1.0f, -1.0f,  1.0f,
              1.0f,  1.0f,  1.0f,
              -1.0f, -1.0f,  1.0f,
              
              -1.0f, -1.0f,  1.0f,
              1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
              
              -1.0f, -1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f, -1.0f, -1.0f,
              
              -1.0f, -1.0f, -1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f, -1.0f,
              
              -1.0f, -1.0f,  1.0f,
              1.0f, -1.0f, -1.0f,
              1.0f, -1.0f,  1.0f,
              
              1.0f, -1.0f, -1.0f,
              -1.0f, -1.0f,  1.0f,
              -1.0f, -1.0f, -1.0f,
              
              -1.0f,  1.0f, -1.0f,
              1.0f,  1.0f,  1.0f,
              1.0f,  1.0f, -1.0f,
              
              1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f,  1.0f
          };
          
          mVertices.clear();
          mUVs.clear();
          mNormals.clear();
          
          for(int i = 0; i < gVertexCount * sizeof(math::vec3); i+=3)
          {
              mVertices.push_back(math::vec3(v[i + 0], v[i + 1], v[i + 2]));
          }
          
          for(int i = 0; i < mVertices.size(); i+=3)
          {
              // compute normal of a triangle
              math::vec3 v0 = mVertices[i + 0];
              math::vec3 v1 = mVertices[i + 1];
              math::vec3 v2 = mVertices[i + 2];
              
              math::vec3 edge0 = v0 - v1;
              math::vec3 edge1 = v2 - v1;
              
              math::vec3 n = math::normalize(math::cross(edge1, edge0));
              
              mNormals.push_back(n);
              mNormals.push_back(n);
              mNormals.push_back(n);
          }
          
      }
  };
}

#endif //__GLPG_PRIMITIVES_H__
