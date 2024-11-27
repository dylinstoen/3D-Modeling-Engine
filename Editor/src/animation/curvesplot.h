/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CURVESPLOT_H
#define CURVESPLOT_H

#include <animator.h>
#include <widgets/QCustomPlot/qcustomplot.h>
#include <animation/controlpoint.h>
#include <animation/curve.h>

class CustomTicker : public QCPAxisTickerFixed {
public:
    void SetTickScaling(unsigned int scaling_factor) { scaling_factor_ = scaling_factor; }
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) override {
//        return QCPAxisTickerFixed::getTickLabel(tick, locale, formatChar, precision);
        return QCPAxisTickerFixed::getTickLabel(tick * scaling_factor_, locale, formatChar, precision);
    }
private:
    unsigned int scaling_factor_;
};

class CurvesPlot : public QCustomPlot, public CurveSamplerFactory {
    Q_OBJECT
public:
    CurvesPlot(QWidget* parent = Q_NULLPTR);

    virtual CurveSampler& CreateCurveSampler() { return AddCurve(); }
    Curve& AddCurve() {
        Curve* curve = new Curve(*this);
        curves_.push_back(std::unique_ptr<Curve>(curve));
        return *curve;
    }

    // We have this notion that we are always "at" some time in the graph
    void MoveCursor(float t, bool step = true);
    float GetCurrentTime() { return cursor_coord_; }

    // Deletes all currently selected control points
    void DeleteControlPoints();

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    void Replot(Curve* curve=nullptr);
    void Rescale();

    // Where t is in seconds
    unsigned int GetAnimationLength() { return animation_length_; }
    void SetAnimationLength(unsigned int t) {
        animation_length_ = t;
        if (cursor_coord_ < 0) MoveCursor(0);
        else if (cursor_coord_ > animation_length_) MoveCursor(animation_length_);
        AnimationLengthChanged.Emit(t);
        Rescale();
    }

    void SetFPS(unsigned int fps) {
        fps_ = fps;
        frame_time_ = 1.0 / fps_;
        half_step_ = frame_time_ / 2.0;
        fixed_ticker_->SetTickScaling(fps_);
        replot();
    }

    // Half the time between two frames
    float GetHalfStep() { return half_step_; }

    Signal1<unsigned int> AnimationLengthChanged;

signals:
    void FrameChanged(float t, unsigned int frame);

protected:
    QSharedPointer<CustomTicker> fixed_ticker_;
    // Keep a set of all active curves
    // std::set<Curve*> active_curves_
    std::vector<std::unique_ptr<Curve>> curves_;
    std::unordered_map<ControlPoint*, ControlPoint*> overlapped_points_;
    float frame_time_; // Time between two frames
    float half_step_; // Half the time between two frames
    float cursor_coord_;
    glm::vec2 previous_mouse_coords_;
    bool moving_points_;
    ControlPoint* hovered_point_;
    QCPItemStraightLine* frame_line_;
    QCPItemTracer* tracer_;

    float RoundToStep(float x) {
        float one_over_step = 1.0 / frame_time_;
        return floor(x * one_over_step + 0.5) / one_over_step;
    }

    std::vector<ControlPoint*> GetSelectedPoints();

    unsigned int animation_length_;
    unsigned int fps_;
};

#endif // CURVESPLOT_H
