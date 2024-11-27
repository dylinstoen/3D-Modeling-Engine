/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "bsplinecurveevaluator.h"

std::vector<glm::vec2> BSplineCurveEvaluator::EvaluateCurve(const std::vector<glm::vec2> &ctrl_pts, int density) const {
    std::vector<glm::vec2> evaluated_pts;

    // REQUIREMENT:
    // Currently this function returns points for a Linear Evaluation.
    // Replace this code with code that returns evaluated points for a B-Spline
    // curve. Be sure to respect the extend_x_ and wrap_ flags.

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

    float tension = 0.5;
    // generate bezier points
    std::vector<glm::vec2> bezier_pts;
    size_t i = 0;
    if (ctrl_pts.size() == 0) {
        if (extend_x_) ExtendX(evaluated_pts, bezier_pts);
        return evaluated_pts;
    } else if (ctrl_pts.size() == 1) {
        AddToBezier(bezier_pts, ctrl_pts[i]);
    } else if (ctrl_pts.size() == 2) {
        AddToBezier(bezier_pts, ctrl_pts[i]);
        AddToBezier(bezier_pts, ctrl_pts[i + 1]);
    } else {
        AddToBezier(bezier_pts, ctrl_pts[i]);
        AddToBezier(bezier_pts, 2.0f * ctrl_pts[i] / 3.0f + ctrl_pts[i + 1] / 3.0f);
        AddToBezier(bezier_pts, ctrl_pts[i] / 3.0f + 2.0f * ctrl_pts[i + 1] / 3.0f);
        for (; i < ctrl_pts.size() - 2; i += 1) {
            AddToBezier(bezier_pts, ctrl_pts[i] / 6.0f + 2.0f * ctrl_pts[i + 1] / 3.0f + ctrl_pts[i + 2] / 6.0f);
            AddToBezier(bezier_pts, 2.0f * ctrl_pts[i + 1] / 3.0f + ctrl_pts[i + 2] / 3.0f);
            AddToBezier(bezier_pts, ctrl_pts[i + 1] / 3.0f + 2.0f * ctrl_pts[i + 2] / 3.0f);
        }
        AddToBezier(bezier_pts, ctrl_pts[i + 1]);
    }

    // draw bezier curve
    i = 3;
    if (bezier_pts.size() >= 4) {
        float interval = 1.0 / (float) density;
        for (; i < bezier_pts.size(); i += 3) {
            float u = 0;
            while (u < 1) {
                float b0 = pow(1 - u, 3);
                float b1 = 3 * u * pow(1 - u, 2);
                float b2 = 3 * pow(u, 2) * (1 - u);
                float b3 = pow(u, 3);
                glm::vec2 b = b0 * bezier_pts[i - 3] + b1 * bezier_pts[i - 2] + b2 * bezier_pts[i - 1] + b3 * bezier_pts[i];
                evaluated_pts.push_back(b);
                u += interval;
            }
        }
    }
    i -= 3;
    // linear interpolation for the rest
    for (; i < bezier_pts.size() - 1; i++) {
        for (int j = 0; j < density; j++) {
            float t = j/(float) density;
            glm::vec2 p = t*bezier_pts[i+1] + (1-t)*bezier_pts[i];
            evaluated_pts.push_back(p);
        }
    }
    evaluated_pts.push_back(bezier_pts.back());
    if (extend_x_) ExtendX(evaluated_pts, bezier_pts);
    return evaluated_pts;
}

void BSplineCurveEvaluator::AddToBezier(std::vector<glm::vec2> &pts, glm::vec2 pt) const {
//    if (pts.empty() || pt[0] > pts.back()[0]) {
        pts.push_back(pt);
        return;
//    }
//    else {
//        pts.push_back(glm::vec2(pts.back()[0], pt[1]));
//        pts.push_back(glm::vec2(pts.back()[0], pt[1]));
//    }
}

