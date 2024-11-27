/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CURVESAMPLER_H
#define CURVESAMPLER_H

#include <animator.h>
#include <animation/keyframe.h>

class CurveSampler {
public:
    static bool AUTOKEY;

    static std::map<CurveType, std::string> TypeToString() {
        static const std::map<CurveType, std::string> type_to_string = {
            {CurveType::Linear, "Linear"},
            {CurveType::Bezier, "Bezier"},
            {CurveType::BSpline, "BSpline"},
            {CurveType::CatmullRom, "CatmullRom"}
        };
        return type_to_string;
    }

    static std::map<std::string, CurveType> StringToType() {
        static const std::map<std::string, CurveType> string_to_type = {
            {"Linear", CurveType::Linear},
            {"Bezier", CurveType::Bezier},
            {"BSpline", CurveType::BSpline},
            {"CatmullRom", CurveType::CatmullRom}
        };
        return string_to_type;
    }

    virtual float SampleAt(float t) const = 0;
    virtual std::vector<Keyframe*> GetKeyframes() const = 0;
    virtual size_t GetKeyframesCount() const = 0;
    virtual void SetKeyframes(const std::vector<float>& t, const std::vector<float>& y) = 0;
    virtual void SetKeyframe(float t, float value) = 0;
    virtual CurveType GetCurveType() const = 0;
    virtual void SetCurveType(CurveType type) = 0;
    virtual bool IsInterpolating() const = 0;
    virtual bool IsWrapping() const = 0;
    virtual void SetWrapping(bool wrap) = 0;
};

class CurveSamplerFactory {
public:
    virtual CurveSampler& CreateCurveSampler() = 0;
};

#endif // CURVESAMPLER_H
