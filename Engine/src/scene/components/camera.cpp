/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "camera.h"
#include <scene/sceneobject.h>

#include <trace/raytracer.h>

REGISTER_COMPONENT(Camera, Camera)

Camera::Camera(double fov, int render_width, int render_height, double near_plane, double far_plane, double width) :
    RenderWidth(true, render_width),
    RenderHeight(true, render_height),
    NearPlane(near_plane),
    FarPlane(far_plane),
    IsPerspective(true),
    FOV(fov),
    Width(width < 0 ? 5.0 : width),
    TraceSettings(),

    TraceRandomMode({"Off", "Uniform Random", "Stratified Random"}, 0),
    TraceDiffuseReflection(true),
    TraceCaustics(true),
    TraceRandomBranching(true),

    TraceSampleCountMode({"Constant", "Adaptive (Recursive)", "Mean Std. Error"}, 0),
    TraceConstantSampleCount({"1", "4", "16", "64", "256", "1024", "4096", "16384"}, 0),
    TraceSampleMinCount({"1", "4", "16", "64", "256"}, 0),
    TraceSampleMaxCount({"4", "16", "64", "256", "1024", "4096", "16384"}, 2),
    TraceAdaptiveSamplingMaxDiff(0.02),
    TraceStdErrorSamplingCutoff(0.02),

    TraceFocusDistance(1.0),
    TraceApertureSize(0.0),
    TraceMaxDepth(true, 5),
    TraceEnableAcceleration(true),
    TraceShadows({"No Shadows", "Opaque Shadows Only", "Translucent Shadows"}, 2),
    TraceEnableReflection(true),
    TraceEnableRefraction(true),

    TraceDebugger()
{
    IsPerspective.Set(true);
    AddProperty("Render Width (px)", &RenderWidth);
    AddProperty("Render Height (px)", &RenderHeight);
    AddProperty("Near Plane", &NearPlane);
    AddProperty("Far Plane", &FarPlane);
    AddProperty("Perspective Camera?", &IsPerspective);
    AddProperty("FOV", &FOV);
    AddProperty("Orthographic View Width", &Width);

    AddProperty("Trace", &TraceSettings);
        TraceSampleSettings.AddProperty("Monte Carlo", &TraceRandomMode);
            TraceSampleSettings.AddProperty("Focus Distance", &TraceFocusDistance);
            TraceSampleSettings.AddProperty("Aperture Size", &TraceApertureSize);
            TraceSampleSettings.AddProperty("Diffuse Reflection", &TraceDiffuseReflection);
            TraceSampleSettings.AddProperty("Caustics", &TraceCaustics);
            TraceSampleSettings.AddProperty("Random Single Branching", &TraceRandomBranching);

        TraceSampleSettings.AddProperty("Sample Count Type", &TraceSampleCountMode);
            TraceSampleSettings.AddProperty("Samples Per Pixel", &TraceConstantSampleCount);
            TraceSampleSettings.AddProperty("Minimum Samples", &TraceSampleMinCount);
            TraceSampleSettings.AddProperty("Maximum Samples", &TraceSampleMaxCount);
            TraceSampleSettings.AddProperty("Difference Threshold", &TraceAdaptiveSamplingMaxDiff);
            TraceSampleSettings.AddProperty("Std. Error Threshold", &TraceStdErrorSamplingCutoff);

        TraceSettings.AddProperty("Maximum Recursion Depth", &TraceMaxDepth);
        TraceSettings.AddProperty("Enable BVH Acceleration", &TraceEnableAcceleration);

        TraceSettings.AddProperty("Shadows", &TraceShadows);
        TraceSettings.AddProperty("Reflections", &TraceEnableReflection);
        TraceSettings.AddProperty("Refractions", &TraceEnableRefraction);

        TraceSettings.AddProperty("Sample Settings", &TraceSampleSettings);
        TraceSettings.AddProperty("Fresnel Term", &TraceFresnel);
        TraceSettings.AddProperty("Beer's Law", &TraceBeers);

        //TraceSettings.AddProperty("Flares Only", &TraceFlaresOnly);

    AddProperty("Trace Debugger", &TraceDebugger);
    for (auto it : std::vector<std::pair<RayType,std::string>>{
            {RayType::reflection, "Show Reflection Rays"},
            {RayType::diffuse_reflection, "Show Diffuse Reflection Rays"},
            {RayType::refraction, "Show Refraction Rays"},
            {RayType::shadow, "Show Shadow Rays"},
            {RayType::hit_normal, "Show Surface Normals"}}) {
        trace_debug_views[(int)it.first] = std::make_unique<BooleanProperty>(true);
        TraceDebugger.AddProperty(it.second, trace_debug_views[(int)it.first].get());
    }

    // Whenever a property changes, we need to recalculate the projection matrix
    RenderWidth.ValueChanged.Connect(this, &Camera::CalculateProjectionI);
    RenderHeight.ValueChanged.Connect(this, &Camera::CalculateProjectionI);
    NearPlane.ValueChanged.Connect(this, &Camera::CalculateProjection);
    FarPlane.ValueChanged.Connect(this, &Camera::CalculateProjection);
    IsPerspective.ValueSet.Connect(this, &Camera::CalculateProjectionB);
    FOV.ValueChanged.Connect(this, &Camera::CalculateProjection);
    Width.ValueChanged.Connect(this, &Camera::CalculateProjection);

    CalculateProjection();

    TraceRandomMode.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesInt);
    TraceSampleCountMode.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesInt);
    TraceDiffuseReflection.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesBool);
    TraceShadows.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesInt);
    TraceEnableReflection.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesBool);
    TraceEnableRefraction.ValueSet.Connect(this, &Camera::UpdateHiddenTracePropertiesBool);
    UpdateHiddenTracePropertiesInt();
}

// Argument is a dummy value to connect to property signals
void Camera::CalculateProjection(double) {
    if (IsPerspective.Get()) {
        projection_ = glm::perspective(glm::radians(FOV.Get()), GetAspectRatio(), NearPlane.Get(), FarPlane.Get());
    } else {
        double w = Width.Get() / 2;
        double h = Width.Get() / GetAspectRatio() / 2;
        projection_ = glm::ortho(-w, w, -h, h, -10.0, 10.0);
    }
}
