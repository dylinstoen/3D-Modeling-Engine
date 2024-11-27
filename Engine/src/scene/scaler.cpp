/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "scaler.h"

void Scaler::ScaleBegin(
        glm::vec3 clicked_point,
        glm::vec3 center,
        glm::vec3 original_scale
        )
{
    original_center_ = center;
    original_scale_ = original_scale;
    t.TranslationBegin(clicked_point, glm::vec4(0,0,0,0));
    freescale = true;
}

void Scaler::AxisScaleBegin(
        glm::vec3 clicked_point,
        glm::vec3 center,
        glm::vec3 scale_axis,
        glm::vec3 original_scale
        )
{
    t.AxisTranslationBegin(clicked_point, scale_axis);
    original_center_ = Translator::ClosestPointOnLine(center, t.GetOriginalPosition(), scale_axis);
    original_scale_ = original_scale;
    freescale = false;
}

void Scaler::PlaneScaleBegin(
        glm::vec3 clicked_point,
        glm::vec3 center,
        glm::vec4 scale_plane,
        glm::vec3 original_scale
        )
{
    original_center_ = center;
    original_scale_ = original_scale;
    t.TranslationBegin(clicked_point, scale_plane);
    freescale = false;
}

void Scaler::ScaleUpdate(int dx, int dy, glm::vec3& out_scale) {
    if (freescale) {
        float d = dx - t.GetOriginalScreenCoords().x;
        float r = 0.01f;
        if (d > 0) d = 1 + d*r;
        else       d = 1/(1-d*r);
        out_scale = glm::vec3(d,d,d);
    } else {
        glm::vec3 translation;
        t.TranslationUpdate(dx, dy, translation);
        glm::vec3 a = t.GetOriginalPosition() - original_center_;
        glm::vec3 b = t.GetNewPosition() - original_center_;
        out_scale = (length(b)/length(a)) * glm::vec3(1,1,1);
    }
}
