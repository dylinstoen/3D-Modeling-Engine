/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef BSPLINECURVEEVALUATOR_H
#define BSPLINECURVEEVALUATOR_H

#include <animation/curveevaluator.h>
#include <animation/beziercurveevaluator.h>

class BSplineCurveEvaluator : public CurveEvaluator {
public:
    BSplineCurveEvaluator() : CurveEvaluator() {}
    BSplineCurveEvaluator(float animation_length, bool wrap)
			: CurveEvaluator(animation_length, wrap) {}

    virtual std::vector<glm::vec2> EvaluateCurve(const std::vector<glm::vec2>& ctrl_pts, int density) const override;
private:
    void AddToBezier(std::vector<glm::vec2> &pts, glm::vec2 pt) const;
};

#endif // BSPLINECURVEEVALUATOR_H
