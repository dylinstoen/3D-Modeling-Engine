/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "curve.h"
#include <animation/keyframe.h>
#include <animation/linearcurveevaluator.h>
#include <animation/beziercurveevaluator.h>
#include <animation/bsplinecurveevaluator.h>
#include <animation/catmullromcurveevaluator.h>
#include <animation/curvesplot.h>
#include <animation/controlpoint.h>
#include <algorithm>

Curve::Curve(CurvesPlot& parent_plot) :
    parent_plot_(&parent_plot),
    graph_(parent_plot.addGraph()),
    tracer_(new QCPItemTracer(&parent_plot)),
    curve_type_(CurveType::Linear),
    visible_(false),
    wrap_curve_(false),
    tension_(0.5f),
    animation_length_(parent_plot.GetAnimationLength())
{
    graph_->setAntialiased(true);
    tracer_->setGraph(graph_);
    tracer_->setStyle(QCPItemTracer::tsNone);
    tracer_->setInterpolating(true);

    // Listen for changes in the animation length
    parent_plot.AnimationLengthChanged.Connect(this, &Curve::OnAnimationLengthChanged);

    GenerateCurve();
    SetVisible(false);
}

float Curve::SampleAt(float t) const {
    tracer_->setGraphKey(t);
    //parent_plot_->replot();
    tracer_->updatePosition(); // much faster than parent_plot_->replot()!
    return tracer_->position->value();
}

std::vector<Keyframe*> Curve::GetKeyframes() const {
    std::vector<Keyframe*> keyframes;
    keyframes.assign(control_points_.begin(), control_points_.end());
    return keyframes;
}

void Curve::SetKeyframes(const std::vector<float> &t, const std::vector<float> &y) {
    control_points_.clear();
    assert(t.size() == y.size());
    // TODO: Duplicate checks
    // TODO: Domain checks
    for (unsigned int i = 0; i < t.size(); i++)
        control_points_.push_back(new ControlPoint(*parent_plot_, this, t[i], y[i]));
    SetVisible(visible_);
    GenerateCurve();
}

void Curve::SetKeyframe(float t, float value) {
    ControlPoint* p = GetControlPoint(t);
    if (p) {
        p->Set(t, value);
    } else {
        p = AddControlPoint(t, value);
    }
}

ControlPoint* Curve::AddControlPoint(float t, float value) {
    // Don't add a control point if one already exists within HALF_STEP of t
    if (GetControlPoint(t) != nullptr) return nullptr;
    ControlPoint* point = new ControlPoint(*parent_plot_, this, t, value);
    point->setVisible(visible_);
    control_points_.push_back(point);
    GenerateCurve();
    return point;
}

void Curve::RemoveControlPoint(ControlPoint *point) {
    if (point == nullptr) return;
    // Find and remove the point from either the control points, or the hidden control points
    auto it = std::find(control_points_.begin(), control_points_.end(), point);
    if (it != control_points_.end()) {
        control_points_.erase(it);
    } else {
        auto it2 = hidden_points_.find(point);
        if (it2 != hidden_points_.end()) hidden_points_.erase(it2);
    }
    // Deletes the control point
    parent_plot_->removeItem(point);
}

void Curve::HideControlPoint(ControlPoint *point) {
    // Make sure the point we're given is actually a shown point
    auto it = std::find(control_points_.begin(), control_points_.end(), point);
    if (it == control_points_.end()) return;
    // Move the point into the list of hidden points and hide it
    hidden_points_.insert(point);
    control_points_.erase(it);
    point->setVisible(false);
    GenerateCurve();
}

void Curve::ShowControlPoint(ControlPoint *point) {
    // Make sure the point we're given is actually a hidden point
    if (point == nullptr) return;
    auto it = hidden_points_.find(point);
    if (it == hidden_points_.end()) return;
    // Move the point back into the list of control points and show it
    control_points_.push_back(point);
    hidden_points_.erase(it);
    point->setVisible(true);
    GenerateCurve();
}

ControlPoint* Curve::GetControlPoint(float t) const {
    // Find the control point from either the control points or the hidden points
    for (auto& ctrl_pt : control_points_)
        if (std::abs(ctrl_pt->Get().x - t) < parent_plot_->GetHalfStep())
            return ctrl_pt;
    for (auto& ctrl_pt : hidden_points_)
        if (std::abs(ctrl_pt->Get().x - t) < parent_plot_->GetHalfStep())
            return ctrl_pt;
    return nullptr;
}

ControlPoint *Curve::GetOverlappingPoint(ControlPoint *point) const {
    // Find the control point from either the control points or the hidden points
    for (auto& ctrl_pt : control_points_)
        if (ctrl_pt != point && std::abs(ctrl_pt->Get().x - point->Get().x) < parent_plot_->GetHalfStep())
            return ctrl_pt;
    for (auto& ctrl_pt : hidden_points_)
        if (ctrl_pt != point && std::abs(ctrl_pt->Get().x - point->Get().x) < parent_plot_->GetHalfStep())
            return ctrl_pt;
    return nullptr;
}

std::vector<ControlPoint *> Curve::GetControlPoints(const QRectF& rect) const {
    std::vector<ControlPoint*> points;

    float left = rect.left();
    float right = left + rect.width();
    float top = rect.top();
    // QRectF expects negative height as the norm, pretty silly
    float bottom = top - rect.height();

    for (auto& ctrl_pt : control_points_) {
        float t = ctrl_pt->Get().x;
        float y = ctrl_pt->Get().y;
        if (t >= left && t <= right && y >= bottom && y <= top)
            points.push_back(ctrl_pt);
    }

    return points;
}

void Curve::GenerateCurve() {
    CurveEvaluator* curve_evaluator = GetCurveEvaluator(curve_type_);
    curve_evaluator->SetTension(tension_);
    std::vector<glm::vec2> evaluated_pts = {glm::vec2(0, 0), glm::vec2(animation_length_, 0)};

    if (control_points_.size() > 0) {
        // TODO: This is a bit inefficient
        std::vector<glm::vec2> ctrl_pts;
        for (auto& ctrl_pt : control_points_) ctrl_pts.push_back(ctrl_pt->Get());
        std::sort(ctrl_pts.begin(), ctrl_pts.end(), [] (const glm::vec2& a, const glm::vec2& b) { return a.x < b.x; });
        evaluated_pts = curve_evaluator->EvaluateCurve(ctrl_pts, 0);
    }

    // Pass the data to the graph
    size_t num_pts = evaluated_pts.size();
    QVector<double> x(num_pts);
    QVector<double> y(num_pts);
    for (unsigned int i = 0; i < num_pts; i++) {
        x[i] = evaluated_pts[i].x;
        y[i] = evaluated_pts[i].y;
    }
    graph_->setData(x, y);

    parent_plot_->replot();
    delete curve_evaluator;
}

void Curve::SetVisible(bool visible) {
    visible_ = visible;
    graph_->setVisible(visible_);
    for (auto& control_point : control_points_) control_point->setVisible(visible_);
}

void Curve::OnAnimationLengthChanged(unsigned int t) {
    animation_length_ = t;
    // Remove any control points that are placed beyond the animation length
    for (size_t i = 0; i < control_points_.size(); i++)
        if (control_points_[i]->Get().x > t) RemoveControlPoint(control_points_[i]);
    for (auto it = hidden_points_.begin(); it != hidden_points_.end(); ) {
        ControlPoint* ctrl_point = *it;
        it++;
        if (ctrl_point->Get().x > t) RemoveControlPoint(ctrl_point);
    }
    GenerateCurve();
}

bool Curve::IsInterpolating() const {
    return curve_type_ != CurveType::Bezier && curve_type_ != CurveType::BSpline;
}

CurveEvaluator* Curve::GetCurveEvaluator(CurveType curve_type) {
    switch (curve_type) {
        case CurveType::Bezier:
            return new BezierCurveEvaluator(animation_length_, wrap_curve_);
            break;
        case CurveType::CatmullRom:
            return new CatmullRomCurveEvaluator(animation_length_, wrap_curve_);
            break;
        case CurveType::BSpline:
            return new BSplineCurveEvaluator(animation_length_, wrap_curve_);
            break;
        case CurveType::Linear:
        default:
            return new LinearCurveEvaluator(animation_length_, wrap_curve_);
            break;
    }
}
