/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SURFACEOFREVOLUTION_H
#define SURFACEOFREVOLUTION_H

#include <properties.h>
#include <scene/components/geometry.h>

class SurfaceOfRevolution : public Geometry {
public:
    FileProperty Curve;
    ChoiceProperty Quality;

    SurfaceOfRevolution();

    virtual Mesh* GetRenderMesh() { return mesh_.get(); }

protected:
    void OnCurveSet(std::string curve_file);
    void OnQualitySet(int quality);
    std::unique_ptr<Mesh> CreateMesh(const std::vector<glm::vec2> &curve_points, unsigned int subdivisions);
    std::unique_ptr<Mesh> mesh_;
    std::vector<std::vector<glm::vec2>> points_list_;
};

#endif // SURFACEOFREVOLUTION_H
