#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <iostream>
#include <glpgGeometry.h>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <algorithm>
#include <glpgMath.h>

using namespace glpg;

int test_sort(std::vector<glm::vec3>& points,
              const geometry::Plane& plane,
              std::vector<glm::vec3>& expected,
              bool verbose = false)
{
    geometry::SortPointsOnPlane(points, plane);
    
    if (points.size() != expected.size())
    {
        std::cout << "Result size does not match expected size" << std::endl;
        return 0;
    }
    
    int test_result = 0;
    
    for(int i = 0; i < expected.size(); ++i)
    {
        if(expected[i] == points[i]) test_result++;
    }
    
    test_result = (test_result == expected.size()) ? 1 : 0;
    
    verbose = !(test_result) || verbose;
    
    if (verbose)
    {
        if(test_result == 0) std::cout << "Result does not match expected:" << std::endl;
        std::cout << "Result - Expected:\n";
        for(int i = 0; i < expected.size(); ++i)
        {
            std::cout << glm::to_string(points[i]) << " ";
            std::cout << glm::to_string(expected[i]) << std::endl;
        }
        std::cout << std::endl;
    }
    
    return test_result;
    
}

int test_center(std::vector<glm::vec3>& points,
                const glm::vec3& expected,
                bool verbose = false)
{
    glm::vec3 center = geometry::ComputeCenter(points);
    
    int test_result = (expected == center) ? 1 : 0;
    
    verbose = !(test_result) || verbose;
    
    if (verbose)
    {
        if(test_result == 0) std::cout << "Result does not match expected:" << std::endl;
        std::cout << "Result: " << glm::to_string(center) << std::endl;
        std::cout << "Expected: " << glm::to_string(expected) << std::endl;
    }
    
    return test_result;
    
}

int test_cube_plane_intersect(const glm::vec3& aabbMin,
                              const glm::vec3& aabbMax,
                              const geometry::Plane& plane,
                              std::vector<glm::vec3>& expected,
                              bool verbose = false)
{
    std::vector<glm::vec3> result;
    
    geometry::ComputePlaneAABBIntersection(aabbMin, aabbMax, plane, result);
    
    if (result.size() != expected.size())
    {
        std::cout << "Result size does not match expected size" << std::endl;
        return 0;
    }
    
    int test_result = 0;
    
    if (std::is_permutation(result.begin(), result.end(), expected.begin(),
                            [&](const glm::vec3& lhs, const glm::vec3& rhs) -> bool {
        
        return (fabs(lhs.x - rhs.x) < GLPG_SMALL_NUM
                && fabs(lhs.y - rhs.y) < GLPG_SMALL_NUM
                && fabs(lhs.z - rhs.z) < GLPG_SMALL_NUM);
//                                lhs == rhs;
        
    })) {
        test_result = 1;
    }
    
//    for(int i = 0; i < expected.size(); ++i)
//    {
//        glm::vec3 p = expected[i];
//        std::vector<glm::vec3>::iterator it;
//        it = std::find(result.begin(), result.end(), p);
//        if(it != result.end()) test_result++;
//    }
    
    verbose = !(test_result) || verbose;
    
    if (verbose)
    {
        if(test_result == 0) std::cout << "Result does not match expected:" << std::endl;
        std::cout << "Result - Expected:\n";
        for(int i = 0; i < result.size(); ++i)
        {
            std::cout << glm::to_string(result[i]) << " ";
            std::cout << glm::to_string(expected[i]) << std::endl;
        }
        std::cout << std::endl;
    }
    
    return test_result;
    
}

int test_segment_plane_interesct(const geometry::Plane& plane,
                                 const geometry::Segment& segment,
                                 geometry::SegmentPlaneIntersection shouldbe,
                                 bool verbose = false)
{
    geometry::SegmentPlaneIntersection i;
    
    geometry::ComputeSegmentPlaneIntersection(segment, plane, i);
    
    bool test_result = (i.result == shouldbe.result);
    
    if (shouldbe.result == geometry::IntersectResult::POINT)
    {
        test_result = (i.point == shouldbe.point);
    }
    
    verbose = !test_result || verbose;
    
    if (i.result == geometry::IntersectResult::POINT && verbose)
    {
        std::cout << glm::to_string(i.point) << std::endl;
    }
    else if(i.result == geometry::IntersectResult::NONE && verbose)
    {
        std::cout << "IntersectResult::NONE" << std::endl;
    }
    else if(i.result == geometry::IntersectResult::SEGMENT && verbose)
    {
        std::cout << "IntersectResult::SEGMENT" << std::endl;
    }
    
    return test_result ? 1 : 0;
    
}

int test_line_plane_interesct(const geometry::Plane& plane,
                              const geometry::Line& line,
                              geometry::LinePlaneIntersection shouldbe,
                              bool verbose = false)
{
    geometry::LinePlaneIntersection i;
    
    geometry::ComputeLinePlaneIntersection(line, plane, i);
    
    bool test_result = (i.result == shouldbe.result);
    
    if (shouldbe.result == geometry::IntersectResult::POINT)
    {
        test_result = (i.point == shouldbe.point);
    }
    
    verbose = !test_result;
    
    if (i.result == geometry::IntersectResult::POINT && verbose)
    {
        std::cout << glm::to_string(i.point) << std::endl;
    }
    else if(i.result == geometry::IntersectResult::NONE && verbose)
    {
        std::cout << "IntersectResult::NONE" << std::endl;
    }
    else if(i.result == geometry::IntersectResult::LINE && verbose)
    {
        std::cout << "IntersectResult::LINE" << std::endl;
    }

    return test_result ? 1 : 0;
    
}

int main(int argc, char** argv)
{
    int tests = 0;
    const int num_tests = 10;
    
    /*
     * Line - Plane intersection
     */
    
    geometry::Plane plane;
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, 1.0f), 2.0f);
    
    geometry::Line line;
    line.point = glm::vec3(0.0f, 0.0f, 1.0f);
    line.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    
    geometry::LinePlaneIntersection expected_lp;
    
    expected_lp.result = geometry::IntersectResult::POINT;
    expected_lp.point = glm::vec3(0.0f, 0.0f, 2.0f);
    
    tests += test_line_plane_interesct(plane, line, expected_lp);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, -1.0f), 2.0f);
    
    line.point = glm::vec3(0.0f, 0.0f, -1.0f);
    line.direction = glm::vec3(0.0f, 0.0f, 1.0f);
    
    expected_lp.result = geometry::IntersectResult::POINT;
    expected_lp.point = glm::vec3(0.0f, 0.0f, -2.0f);
    
    tests += test_line_plane_interesct(plane, line, expected_lp);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, -1.0f), 4.0f);
    
    line.point = glm::vec3(0.0f, 0.0f, -4.0f);
    line.direction = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    
    expected_lp.result = geometry::IntersectResult::LINE;
    
    tests += test_line_plane_interesct(plane, line, expected_lp);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, -1.0f), -4.0f);
    
    line.point = glm::vec3(0.0f, 0.0f, 0.0f);
    line.direction = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    
    expected_lp.result = geometry::IntersectResult::NONE;
    
    tests += test_line_plane_interesct(plane, line, expected_lp);

    
    /*
     * Plane - Segment intersection
     */
    geometry::Segment segment;
    geometry::SegmentPlaneIntersection expected_sp;
    
    segment.A = glm::vec3( 0.0f, 0, -1.0f);
    segment.B = glm::vec3( 0.0f, 0,  1.0f);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, 1.0f), 0.5f);
    
    expected_sp.result = geometry::IntersectResult::POINT;
    expected_sp.point = glm::vec3(0,0,0.5f);
    
    tests += test_segment_plane_interesct(plane, segment, expected_sp);
    
    segment.A = glm::vec3( 0.0f, 0,  1.0f);
    segment.B = glm::vec3( 0.0f, 0, -1.0f);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, 1.0f), 0.5f);
    
    tests += test_segment_plane_interesct(plane, segment, expected_sp);
    
    segment.A = glm::vec3( 0.0f, 0, 0.5f);
    segment.B = glm::vec3( 0.0f, 0, 0.5f);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, 1.0f), 0.5f);
    
    expected_sp.result = geometry::IntersectResult::POINT;
    expected_sp.point = glm::vec3(0,0,0.5f);
    
    tests += test_segment_plane_interesct(plane, segment, expected_sp);
    
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    std::vector<glm::vec3> expected_ci;
    
    aabbMin = glm::vec3(0,0,0);
    aabbMax = glm::vec3(1,1,1);
    
    plane = geometry::planeFromNormalAndDistance(glm::vec3(0.0f, 0.0f, 1.0f), 0.5f);
    
    expected_ci.push_back(glm::vec3(0,0,0.5));
    expected_ci.push_back(glm::vec3(1,0,0.5));
    expected_ci.push_back(glm::vec3(0,1,0.5));
    expected_ci.push_back(glm::vec3(1,1,0.5));
    
    tests += test_cube_plane_intersect(aabbMin, aabbMax, plane, expected_ci);
    
    // A more complex test, generated in Blender
//    glm::vec3 pp0(1.78059, 0.36637, 1.3103);
//    glm::vec3 pp1(0.36637, 1.78059, 1.3103);
//    glm::vec3 pp2(0.78059, -0.63363, -0.10392);
//    
//    plane.point = pp0;
//    plane.normal = glm::cross(pp1 - pp0, pp2 - pp0);
//    
//    expected_ci.clear();
//    expected_ci.push_back(glm::vec3(0.293924004f, 0.000000f, 0.000000f));
//    expected_ci.push_back(glm::vec3(0.708131015f, 1.000000f, 1.000000f));
//    expected_ci.push_back(glm::vec3(0.000000f, 0.293924004f, 0.000000f));
//    expected_ci.push_back(glm::vec3(1.000000f, 0.708131015f, 1.000000f));
//    expected_ci.push_back(glm::vec3(1.000000f, 0.000000f, 0.499273002f));
//    expected_ci.push_back(glm::vec3(0.000000f, 1.000000f, 0.499273002f));
//    
//    tests += test_cube_plane_intersect(aabbMin, aabbMax, plane, expected_ci);
    
    std::vector<glm::vec3> points;
    glm::vec3 expected_center(0.5f, 0.5f, 0.0f);
    
    points.push_back(glm::vec3(0,0,0));
    points.push_back(glm::vec3(1,0,0));
    points.push_back(glm::vec3(0,1,0));
    points.push_back(glm::vec3(1,1,0));
    
    tests += test_center(points, expected_center);
    
    points.clear();
    
    points.push_back(glm::vec3(0.169f, 1.43934f, 0));
    points.push_back(glm::vec3(-0.41878f, -0.36968f, 0));
    points.push_back(glm::vec3(-0.78205f, 0.74836f, 0));
    points.push_back(glm::vec3(0.75679f, -0.36968f, 0));
    points.push_back(glm::vec3(1.12006f, 0.74836f, 0));
    points.push_back(glm::vec3(0.75679f, -0.36968f, 0));
    
    std::vector<glm::vec3> sorted;
    sorted.push_back(glm::vec3(0.169f, 1.43934f, 0));
    sorted.push_back(glm::vec3(1.12006f, 0.74836f, 0));
    sorted.push_back(glm::vec3(0.75679f, -0.36968f, 0));
    sorted.push_back(glm::vec3(-0.41878f, -0.36968f, 0));
    sorted.push_back(glm::vec3(-0.78205f, 0.74836f, 0));
    
    plane.point = glm::vec3(0,0,0);
    plane.point = glm::vec3(0,0,1);
    
    tests += test_sort(points, plane, sorted);
    
    
    std::cout << "\n******************" << std::endl;
    std::cout << "Num Tests:    " << num_tests << std::endl;
    std::cout << "PASS:         " << tests << std::endl;
    std::cout << "FAIL:         " << num_tests - tests << std::endl;
    std::cout << "******************\n" << std::endl;
    
    return 0;
}