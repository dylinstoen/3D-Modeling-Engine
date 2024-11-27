/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "doubleproperty.h"

#include <scene/scenemanager.h>
#include <QDebug>

DoubleProperty::DoubleProperty(double value) :
    Property(),
    value_(value)
{
}

DoubleProperty::DoubleProperty(double value, double min, double max, double step) :
    Property(),
    value_(value)
{
    range_ = new RangePropertyData();
    range_->min = min;
    range_->max = max;
    range_->step = step;
}

DoubleProperty::~DoubleProperty() {
    if (range_) {
        delete range_;
    }

    if (curve_) {
        delete curve_;
    }
}

double DoubleProperty::Get() const {
    return value_;
}

void DoubleProperty::Set(double value) {
    if (std::abs(value_ - value) <= std::numeric_limits<double>::epsilon()) return;
    value_ = value;
    if (curve_ && curve_->IsInterpolating() && CurveSampler::AUTOKEY) {
        curve_->SetKeyframe(frame_, value_);
        CurveUpdated.Emit();
    }
    if (allow_signals_) ValueChanged.Emit(value);
}

void DoubleProperty::SetAnimationTime(float t) {
    frame_ = t;
    if (curve_ && curve_->GetKeyframesCount() > 0) {
        float value = curve_->SampleAt(t);
        if (std::abs(value_ - value) < std::numeric_limits<double>::epsilon()) return;
        value_ = value;
        if (allow_signals_) ValueChanged.Emit(value);
    }
}

void DoubleProperty::SaveToYAML(YAML::Emitter& out) const {
    if (curve_ && curve_->GetKeyframesCount() > 1) {
        out << YAML::BeginMap;
        out << YAML::Key << "Interpolation" << YAML::Value << CurveSampler::TypeToString()[curve_->GetCurveType()];
        out << YAML::Key << "Wrap" << YAML::Value << curve_->IsWrapping();
        out << YAML::Key << "FrameTimes" << YAML::Flow << YAML::BeginSeq;
        for (auto& keyframe : curve_->GetKeyframes()) {
            out << keyframe->Get().x;
        }
        out << YAML::EndSeq;
        out << YAML::Key << "FrameValues" << YAML::Flow << YAML::BeginSeq;
        for (auto& keyframe : curve_->GetKeyframes()) {
            out << keyframe->Get().y;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;
    } else if (curve_ && curve_->GetKeyframesCount() == 1) {
        out << curve_->GetKeyframes()[0]->Get().y;
    } else {
        out << value_;
    }
}

void DoubleProperty::LoadFromYAML(const YAML::Node& node) {
    if (curve_ != nullptr) {
        delete curve_;
        curve_ = nullptr;
    }
    if (node.IsMap()) {
        std::vector<float> t;
        std::vector<float> y;
        if (node["FrameTimes"]) {
            assert(node["FrameValues"] && node["FrameTimes"].size() == node["FrameValues"].size());
            const YAML::Node& frame_times = node["FrameTimes"];
            const YAML::Node& frame_values = node["FrameValues"];
            for (size_t i = 0; i < frame_times.size(); i++) {
                t.push_back(frame_times[i].as<float>());
            }
            for (size_t i = 0; i < frame_values.size(); i++) {
                y.push_back(frame_values[i].as<float>());
            }
        }

        if (y.size() > 1) {
            //MAKING THE CURVE CAUSES THE SCREEN TO REFRESH
            curve_ = &(SceneManager::Instance()->GetCurveSamplerFactory()->CreateCurveSampler());
            if (node["Interpolation"]) curve_->SetCurveType(CurveSampler::StringToType()[node["Interpolation"].as<std::string>()]);
            if (node["Wrap"]) curve_->SetWrapping(node["Wrap"].as<bool>());
            curve_->SetKeyframes(t, y);
            SetAnimationTime(0);
        } else {
            value_ = y.size()==1 ? y[0] : 0;
            if (allow_signals_) ValueChanged.Emit(value_);
        }
    } else {
        value_ = node.as<double>();
        if (allow_signals_) ValueChanged.Emit(value_);
    }
}
