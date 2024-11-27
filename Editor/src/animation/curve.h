/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CURVE_H
#define CURVE_H

#include <animator.h>
#include <animation/curvesampler.h>
#include <animation/curveevaluator.h>

class QRectF;
class CurvesPlot;
class ControlPoint;
class QCPGraph;
class QCPItemTracer;

// TODO: Add discrete curves that use integral values of y.
// TODO: Add binary curves that only use values 0 and 1 for y.
class Curve : public CurveSampler {
public:
    // Curve is hidden by default
    Curve(CurvesPlot& parent_plot);

    // Samples the splined curve at time t.
    virtual float SampleAt(float t) const override;

    // Returns a vector of Control Points representing Keyframe<time, value>
    virtual std::vector<Keyframe*> GetKeyframes() const;

    // Returns the number of shown Control Points
    virtual size_t GetKeyframesCount() const { return control_points_.size(); }

    // Sets the Control Points of the curve to Keyframes<time, value>.
    virtual void SetKeyframes(const std::vector<float>& t, const std::vector<float>& y);

    // Sets (or creates) keyframe at time t to have given value
    virtual void SetKeyframe(float t, float value);

    // Adds a new control point to the curve at the given time t.
    // Does nothing if another control point already exists within HALF_STEP of t.
    ControlPoint* AddControlPoint(float t, float value);

    // Deletes the control point from the curve and the plot.
    // The control point should not be used again after this method.
    void RemoveControlPoint(ControlPoint* point);

    // Hides the control point from the curve.
    // Does nothing if the control point is not a shown point belonging to the curve.
    void HideControlPoint(ControlPoint* point);

    // Show the hidden control point.
    // Does nothing if the control point is not a hidden point belonging to the curve.
    void ShowControlPoint(ControlPoint* point);

    // Returns a control point (if any) that is within HALF_STEP of t.
    // The control point can be either a hidden point, or a shown point.
    ControlPoint* GetControlPoint(float t) const;

    // Returns another control point (if any) that shares the same t within HALF_STEP as point
    // The control point can be either a hidden point, or a shown point.
    ControlPoint* GetOverlappingPoint(ControlPoint* point) const;

    // Returns the control points (if any) that lie inside the domain and range defined by the rectangle.
    // Does not look for hidden points.
    std::vector<ControlPoint*> GetControlPoints(const QRectF& rect) const;

    // Regenerates the splined curve based on its control points using the current curve type.
    void GenerateCurve();

    // Which type of spline this curve represents.
    virtual CurveType GetCurveType() const override { return curve_type_; }
    virtual void SetCurveType(CurveType curve_type) override {
        curve_type_ = curve_type;
        GenerateCurve();

    }

    virtual bool IsInterpolating() const;
    // Whether or not to wrap the curve back around to the front.
    virtual bool IsWrapping() const override { return wrap_curve_; }
    virtual void SetWrapping(bool wrap) override { wrap_curve_ = wrap; GenerateCurve(); }

    // Whether or not this curve is visible on the graph.
    void SetVisible(bool visible);

    float  GetTension() { return tension_; }
    void SetTension(float tension) { tension_ = tension; GenerateCurve(); }

protected:
    std::vector<ControlPoint*> control_points_;
    std::set<ControlPoint*> hidden_points_;
    CurvesPlot* parent_plot_;
    QCPGraph* graph_;
    QCPItemTracer* tracer_;
    CurveType curve_type_;
    bool visible_;
    bool wrap_curve_;
    float tension_;
    unsigned int animation_length_; // Seconds

    // Called when the plot changes the animation length
    void OnAnimationLengthChanged(unsigned int t);

    // Returns the curve evaluator for the splined curve type.
    CurveEvaluator* GetCurveEvaluator(CurveType curve_type);
};
#endif // CURVE_H
