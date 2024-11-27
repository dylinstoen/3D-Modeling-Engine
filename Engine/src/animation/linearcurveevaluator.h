/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef LINEARCURVEEVALUATOR_H
#define LINEARCURVEEVALUATOR_H

#include <animation/curveevaluator.h>

class LinearCurveEvaluator : public CurveEvaluator {
public:
    LinearCurveEvaluator() : CurveEvaluator() {}
    LinearCurveEvaluator(float animation_length, bool wrap)
			: CurveEvaluator(animation_length, wrap) {}

    virtual std::vector<glm::vec2> EvaluateCurve(const std::vector<glm::vec2>& ctrl_pts, int density) const override {
        if (density == 0) density = 100;
        std::vector<glm::vec2> evaluated_pts;
        for (size_t i = 0; i < ctrl_pts.size()-1; i++) {
			for (int j = 0; j < density; j++) {
                float t = j/(float) density;
                glm::vec2 p = t*ctrl_pts[i+1] + (1-t)*ctrl_pts[i];
                evaluated_pts.push_back(p);
			}
		}
        evaluated_pts.push_back(ctrl_pts.back());
        if (extend_x_) ExtendX(evaluated_pts, ctrl_pts);
        return evaluated_pts;
    }
};

#endif // LINEARCURVEEVALUATOR_H
