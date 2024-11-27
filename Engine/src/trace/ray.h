/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RAY_H
#define RAY_H

#include <vectors.h>

const double RAY_EPSILON = 0.001;
const double NORMAL_EPSILON = 0.00001;
const double EDGE_EPSILON = 0.000000001;
const double INDEX_OF_AIR = 1.0003;

class TraceSceneObject;
class Material;

// A ray has a position where the ray starts, and a direction (which should
// always be normalized!)

class Ray {
public:
    /* enum RayType
	{
		VISIBILITY,
       MIRROR,
		REFRACTION,
       REFLECTION,
		SHADOW
	};

     glm::dvec2 da;
    glm::dvec2 ds;
    glm::dvec2 dr; */

    Ray( const glm::dvec3& pp, const glm::dvec3& dd ) :
          position( pp ), direction(dd)  //, t( tt )
    {
        //assert(glm::length(dd) > 0.999 && glm::length(dd) < 1.001); //expects a normalized input
    }
	~Ray() {}

    Ray& operator =( const Ray& other )
    { position = other.position; direction = other.direction; return *this; }

    glm::dvec3 at( double t ) const
    { return position + (t*direction); }

    //RayType t;

    glm::dvec3 position;
    glm::dvec3 direction;
};


// The description of an intersection point.
class Intersection {
  public:
    Intersection() {} // : obj( nullptr ), t( 0.0 ), N(), material(0) {}
    ~Intersection() {}
    
    TraceSceneObject* obj;
    double t;
    glm::vec3 normal;
    glm::vec2 uv;
    Material* GetMaterial();
    glm::vec3 GetTrueNormal();
};

#endif // __RAY_H__
