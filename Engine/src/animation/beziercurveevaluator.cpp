/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "beziercurveevaluator.h"

std::vector<glm::vec2> BezierCurveEvaluator::EvaluateCurve(const std::vector<glm::vec2> &ctrl_pts, int density) const {
    std::vector<glm::vec2> evaluated_pts;

    // REQUIREMENT:
    // Currently this function returns points for a Linear Evaluation.
    // Replace this code with code that returns evaluated points for a Bezier
    // Spline curve. Be sure to respect the extend_x_ and wrap_ flags in a
    // a reasonable way.

//    if (density == 0) density = 100;
//    for (size_t i = 0; i < ctrl_pts.size()-1; i++) {
//        for (int j = 0; j < density; j++) {
//            float t = j/(float) density;
//            glm::vec2 p = t*ctrl_pts[i+1] + (1-t)*ctrl_pts[i];
//            evaluated_pts.push_back(p);
//        }
//    }
//    evaluated_pts.push_back(ctrl_pts.back());
//    if (extend_x_) ExtendX(evaluated_pts, ctrl_pts);
//    return evaluated_pts;
    if (density == 0) density = 100;
    size_t i = 3;
    if (ctrl_pts.size() >= 4) {
        float interval = 1.0 / (float) density;
        for (; i < ctrl_pts.size(); i += 3) {
            float u = 0;
            while (u < 1) {
                float b0 = pow(1 - u, 3);
                float b1 = 3 * u * pow(1 - u, 2);
                float b2 = 3 * pow(u, 2) * (1 - u);
                float b3 = pow(u, 3);
                glm::vec2 b = b0 * ctrl_pts[i - 3] + b1 * ctrl_pts[i - 2] + b2 * ctrl_pts[i - 1] + b3 * ctrl_pts[i];
                evaluated_pts.push_back(b);
                u += interval;
            }
        }
    }
    i -= 3;
    // linear interpolation for the rest
    for (; i < ctrl_pts.size() - 1; i++) {
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
