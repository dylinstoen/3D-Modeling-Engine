/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef DOUBLEPROPERTY_H
#define DOUBLEPROPERTY_H

#include <properties/property.h>
#include <animation/curvesampler.h>

struct RangePropertyData {
    double min;
    double max;
    double step;
};

class DoubleProperty : public Property {
public:
    Signal1<double> ValueChanged;
    Signal0<> CurveUpdated;

    DoubleProperty(double value = 0.0);
    DoubleProperty(double value, double min, double max, double step);
    ~DoubleProperty();

    double Get() const;
    void Set(double value);

    CurveSampler* GetCurve() { return curve_; }
    void SetCurve(CurveSampler* curve) { curve_ = curve; }
    void SetAnimationTime(float frame);

    bool IsRange() const { return range_!=nullptr; }
    double GetMin() const { return range_->min; }
    double GetMax() const { return range_->max; }
    double GetStep() const { return range_->step; }

    virtual void SaveToYAML(YAML::Emitter& out) const override;
    virtual void LoadFromYAML(const YAML::Node& node) override;

private:
    double value_;
    float frame_;
    RangePropertyData* range_ = nullptr;
    CurveSampler* curve_ = nullptr;
};

#endif // DOUBLEPROPERTY_H
