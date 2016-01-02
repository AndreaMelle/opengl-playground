//
//  glpgGeometry.h
//  opengl-playground
//
//  Created by Andrea Melle on 29/12/2015.
//
//

#ifndef glpgGeometry_h
#define glpgGeometry_h

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <unordered_set>
#include <glpgMath.h>

namespace glpg
{
namespace geometry
{
  
    typedef enum IntersectResult
    {
        LINE, // if the line lies on plane, the intersection domain is a line
        NONE, // parallel and not on plane
        POINT, // intersection is a point
        SEGMENT // intersection is a segment
    } IntersectResult;
    
    typedef struct Line
    {
        glm::vec3 point;
        glm::vec3 direction; // must be normalized
    } Line;
    
    typedef struct Segment
    {
        glm::vec3 A;
        glm::vec3 B; // must be normalized
    } Segment;
    
    typedef struct Plane
    {
        glm::vec3 point; //signed distance from origin
        glm::vec3 normal; //must be normalized
    } Plane;
    
    Plane planeFromNormalAndDistance(const glm::vec3& normal, const float d)
    {
        Plane p;
        p.normal = normal;
        p.point = d * normal;
        return p;
    }
    
    typedef struct LinePlaneIntersection
    {
        IntersectResult result;
        float t; //parameter, useful for segment bounds checking
        glm::vec3 point;
    } LinePlaneIntersection;
    
    typedef struct SegmentPlaneIntersection
    {
        IntersectResult result;
        glm::vec3 point;
    } SegmentPlaneIntersection;
    
    IntersectResult ComputeLinePlaneIntersection(const Line& line,
                                                 const Plane& plane,
                                                 LinePlaneIntersection& result)
    {
        float NdotV, NdotSplusd;
        
        NdotV = glm::dot(line.direction, plane.normal);
        NdotSplusd = glm::dot(line.point - plane.point, plane.normal);// + plane.d;
        
        //  NdotV == 0 - avoid division overflow later in division
        if(fabsf(NdotV) < GLPG_SMALL_NUM)
        {
            if(NdotSplusd == 0) result.result = IntersectResult::LINE;
            else result.result = IntersectResult::NONE;
        }
        else
        {
            result.t = - NdotSplusd / NdotV;
            result.point = line.point + result.t * line.direction;
            result.result = IntersectResult::POINT;
        }
        
        return result.result;
    }
    
    IntersectResult ComputeSegmentPlaneIntersection(const Segment& segment,
                                                    const Plane& plane,
                                                    SegmentPlaneIntersection& result)
    {
        if (segment.A == segment.B)
        {
            // just test if the point is on plane
            float NdotSplusd = glm::dot(segment.A - plane.point, plane.normal);
            if(NdotSplusd == 0)
            {
                result.result = IntersectResult::POINT;
                result.point = segment.A;
            }
        }
        
        Line line;
        LinePlaneIntersection lres;
        
        line.point = segment.A;
        line.direction = segment.B - segment.A;
        
        ComputeLinePlaneIntersection(line, plane, lres);
        
        if (lres.result == IntersectResult::POINT && lres.t >= 0 && lres.t <= 1.0f)
        {
            result.point = lres.point;
            result.result = IntersectResult::POINT;
        }
        else if (lres.result == IntersectResult::LINE)
            result.result = IntersectResult::SEGMENT;
        else
            result.result = IntersectResult::NONE;
        
        return result.result;
    }
    
    void HandleAABBSegmentPlaneIntersection(const SegmentPlaneIntersection& spi,
                                            const Segment& s,
                                            std::vector<glm::vec3>& points)
    {
        if (spi.result == IntersectResult::POINT)
        {
            points.push_back(spi.point);
        }
        else if (spi.result == IntersectResult::LINE)
        {
            points.push_back(s.A);
            points.push_back(s.B);
        }
    }
    
    int ComputePlaneAABBIntersection(const glm::vec3& aabbMin,
                                     const glm::vec3& aabbMax,
                                     const Plane& plane,
                                     std::vector<glm::vec3>& points)
    {
        points.clear();
        
        Segment s;
        SegmentPlaneIntersection spi;
        float xdim, ydim, zdim;
        
        xdim = aabbMax.x - aabbMin.x;
        ydim = aabbMax.y - aabbMin.y;
        zdim = aabbMax.z - aabbMin.z;
        
        // Along the x axis
        s.A = aabbMin;
        s.B = s.A + glm::vec3(xdim, 0, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(0, 0, zdim);
        s.B = s.A + glm::vec3(xdim, 0, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(0, ydim, 0);
        s.B = s.A + glm::vec3(xdim, 0, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(0, ydim, zdim);
        s.B = s.A + glm::vec3(xdim, 0, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        // Along the y axis
        s.A = aabbMin;
        s.B = s.A + glm::vec3(0, ydim, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(0, 0, zdim);
        s.B = s.A + glm::vec3(0, ydim, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(xdim, 0, 0);
        s.B = s.A + glm::vec3(0, ydim, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(xdim, 0, zdim);
        s.B = s.A + glm::vec3(0, ydim, 0);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        // Along the z axis
        s.A = aabbMin;
        s.B = s.A + glm::vec3(0, 0, zdim);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(xdim, 0, 0);
        s.B = s.A + glm::vec3(0, 0, zdim);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(0, ydim, 0);
        s.B = s.A + glm::vec3(0, 0, zdim);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        s.A = aabbMin + glm::vec3(xdim, ydim, 0);
        s.B = s.A + glm::vec3(0, 0, zdim);
        ComputeSegmentPlaneIntersection(s, plane, spi);
        HandleAABBSegmentPlaneIntersection(spi, s, points);
        
        return points.size();
    }
    
    // Sort points lying on a plane and removes duplicates
    // TODO: add option to sort in CW or CCW order
    void SortPointsOnPlane(std::vector<glm::vec3>& points, const Plane& plane)
    {
        if (points.size() == 0) return;
        
        
        
        const glm::vec3 origin = points[0];
        
        std::sort(points.begin(), points.end(), [&, origin, plane](const glm::vec3& lhs, const glm::vec3& rhs) -> bool {
            
            glm::vec3 v = glm::cross((lhs - origin), (rhs - origin));
            return glm::dot(v, plane.normal) < 0;
            
        });
        
        points.erase(std::unique(points.begin(), points.end(), [&](const glm::vec3& lhs, const glm::vec3& rhs) -> bool {
            
            return lhs == rhs;
            
        }), points.end());
    }
    
    // Compute center of set of unique points
    glm::vec3 ComputeCenter(const std::vector<glm::vec3>& points)
    {
        assert(points.size() != 0);
        
        glm::vec3 center = points[0];
        
        for(int i = 1; i < points.size(); ++i)
            center += points[i];
        
        return center / (float)points.size();
    }
    
};
};


#endif /* glpgGeometry_h */
