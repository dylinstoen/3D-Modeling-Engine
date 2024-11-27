/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CAMERA_H
#define CAMERA_H

#include <scene/components/component.h>
#include <trace/raytracer.h>
enum RayType;

struct DebugRay {
  RayType type;
  glm::vec3 p1;
  glm::vec3 p2;
};

class Camera : public Component {
public:
    // Camera Properties
    IntProperty RenderWidth;
    IntProperty RenderHeight;
    DoubleProperty NearPlane;
    DoubleProperty FarPlane;
    BooleanProperty IsPerspective;
    // Perspective Camera Properties
    DoubleProperty FOV;
    // Orthographic Camera Properties
    DoubleProperty Width;

    PropertyGroup TraceSettings;
    PropertyGroup TraceSampleSettings;
    PropertyGroup TraceDebugger;

    // Trace stuff
    ChoiceProperty TraceRandomMode;
        static const int TRACERANDOM_DETERMINISTIC = 0;

        static const int TRACERANDOM_UNIFORM = 1;
        static const int TRACERANDOM_STRATIFIED = 2;
        BooleanProperty TraceDiffuseReflection;
        BooleanProperty TraceCaustics;
        BooleanProperty TraceRandomBranching;
        DoubleProperty TraceFocusDistance;
        DoubleProperty TraceApertureSize;

    ChoiceProperty TraceSampleCountMode;
        static const int TRACESAMPLING_CONSTANT = 0;
        ChoiceProperty TraceConstantSampleCount;

        static const int TRACESAMPLING_RECURSIVE = 1;
        static const int TRACESAMPLING_STDERROR = 2;
        ChoiceProperty TraceSampleMinCount;
        ChoiceProperty TraceSampleMaxCount;
        DoubleProperty TraceAdaptiveSamplingMaxDiff;
        DoubleProperty TraceStdErrorSamplingCutoff;

    //BooleanProperty TraceFlaresOnly;
    IntProperty TraceMaxDepth;
    BooleanProperty TraceEnableAcceleration;
    static const int TRACESHADOWS_NONE = 0;
    static const int TRACESHADOWS_OPAQUE = 1;
    static const int TRACESHADOWS_COLORED = 2;
    ChoiceProperty TraceShadows;
    BooleanProperty TraceEnableReflection;
    BooleanProperty TraceEnableRefraction;
    BooleanProperty TraceFresnel;
    BooleanProperty TraceBeers;

    std::map<int, std::unique_ptr<BooleanProperty>> trace_debug_views;

    Camera(double fov = 50.0f, int render_width = 1280, int render_height = 720, double near_plane = 0.1f, double far_plane = 100.0f, double width = 5.0f);

    // Returns the Camera's projection matrix
    glm::mat4 GetProjection() const { return projection_; }
    // Returns the Camera's aspect ratio
    double GetAspectRatio() const { return double(RenderWidth.Get()) / RenderHeight.Get(); }

    void ClearDebugRays() { debug_rays_.clear(); }

    void AddDebugRay(glm::dvec3 p1, glm::dvec3 p2, RayType rtype) {
        debug_rays_.push_back(DebugRay{rtype,p1,p2});
    }

    void UpdateHiddenTracePropertiesInt(int aaa=0) {
        TraceDiffuseReflection.SetHidden(TraceRandomMode.Get()==TRACERANDOM_DETERMINISTIC && TraceEnableReflection.Get());
        TraceCaustics.SetHidden(TraceRandomMode.Get()==TRACERANDOM_DETERMINISTIC || !TraceDiffuseReflection.Get() || TraceShadows.Get()==Camera::TRACESHADOWS_NONE);
        TraceRandomBranching.SetHidden(TraceRandomMode.Get()==TRACERANDOM_DETERMINISTIC);
        TraceFocusDistance.SetHidden(TraceRandomMode.Get()==TRACERANDOM_DETERMINISTIC);
        TraceApertureSize.SetHidden(TraceRandomMode.Get()==TRACERANDOM_DETERMINISTIC);
        TraceConstantSampleCount.SetHidden(TraceSampleCountMode.Get()!=TRACESAMPLING_CONSTANT);
        TraceSampleMinCount.SetHidden(TraceSampleCountMode.Get()==TRACESAMPLING_CONSTANT);
        TraceSampleMaxCount.SetHidden(TraceSampleCountMode.Get()==TRACESAMPLING_CONSTANT);
        TraceAdaptiveSamplingMaxDiff.SetHidden(TraceSampleCountMode.Get()!=TRACESAMPLING_RECURSIVE);
        TraceStdErrorSamplingCutoff.SetHidden(TraceSampleCountMode.Get()!=TRACESAMPLING_STDERROR);
        TraceShadows.SetHidden(true);
        TraceEnableReflection.SetHidden(true);
        TraceEnableRefraction.SetHidden(true);

    }

    void UpdateHiddenTracePropertiesBool(bool aaa=false) {
        UpdateHiddenTracePropertiesInt(0);
    }

    std::vector<DebugRay> debug_rays_;

protected:
    glm::mat4 projection_;
    // Recalculates the camera's projection matrix and stores it in projection_
    void CalculateProjection(double dummy = 0.0f);
    void CalculateProjectionB(bool) { CalculateProjection(); }
    void CalculateProjectionI(int) { CalculateProjection(); }
};

#endif // CAMERA_H
