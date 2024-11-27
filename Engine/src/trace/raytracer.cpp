/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "trace/raytracer.h"
#include <scene/scene.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <glm/gtx/component_wise.hpp>
#include <scene/components/triangleface.h>
#include <glm/gtx/string_cast.hpp>
#include "components.h"

unsigned int pow4(unsigned int e) {
    unsigned int i = 1;
    while(e>0) {
        i *= 4;
        e--;
    }
    return i;
}

RayTracer::RayTracer(Scene& scene, SceneObject& camobj) :
    trace_scene(&scene, camobj.GetComponent<Camera>()->TraceEnableAcceleration.Get()), next_render_index(0), cancelling(false), first_pass_buffer(nullptr)
{
    Camera* cam = camobj.GetComponent<Camera>();

    settings.width = cam->RenderWidth.Get();
    settings.height = cam->RenderHeight.Get();
    settings.pixel_size_x = 1.0/double(settings.width);
    settings.pixel_size_y = 1.0/double(settings.height);

    settings.shadows = cam->TraceShadows.Get() != Camera::TRACESHADOWS_NONE;
    settings.translucent_shadows = cam->TraceShadows.Get() == Camera::TRACESHADOWS_COLORED;
    settings.reflections = cam->TraceEnableReflection.Get();
    settings.refractions = cam->TraceEnableRefraction.Get();
    settings.fresnel = cam->TraceFresnel.Get();
    settings.beers = cam->TraceBeers.Get();

    settings.random_mode = cam->TraceRandomMode.Get();
    settings.diffuse_reflection = cam->TraceEnableReflection.Get() && cam->TraceDiffuseReflection.Get() && settings.random_mode != Camera::TRACERANDOM_DETERMINISTIC;
    settings.caustics = settings.diffuse_reflection && settings.shadows && cam->TraceCaustics.Get();
    settings.random_branching = cam->TraceRandomBranching.Get() && settings.random_mode != Camera::TRACERANDOM_DETERMINISTIC;

    settings.samplecount_mode = cam->TraceSampleCountMode.Get();

    settings.constant_samples_per_pixel = pow4(cam->TraceConstantSampleCount.Get());
    settings.dynamic_sampling_min_depth = cam->TraceSampleMinCount.Get();
    settings.dynamic_sampling_max_depth = cam->TraceSampleMaxCount.Get()+1;
    settings.adaptive_max_diff_squared = cam->TraceAdaptiveSamplingMaxDiff.Get();
    settings.adaptive_max_diff_squared *= settings.adaptive_max_diff_squared;
    settings.max_stderr = cam->TraceStdErrorSamplingCutoff.Get();

    if (settings.samplecount_mode == Camera::TRACESAMPLING_RECURSIVE && settings.random_mode!=Camera::TRACERANDOM_DETERMINISTIC) {
        qDebug() << "Adaptive Recursive Supersampling does not work with Monte Carlo!";
        settings.samplecount_mode = Camera::TRACESAMPLING_CONSTANT;
    }

    settings.max_depth = cam->TraceMaxDepth.Get();

    //camera looks -z, x is right, y is up
    glm::mat4 camera_matrix = camobj.GetModelMatrix();

    settings.projection_origin = glm::vec3(camera_matrix * glm::vec4(0,0,0,1));

    glm::dvec3 fw_point = glm::dvec3(camera_matrix * glm::vec4(0,0,-1,1));
    glm::dvec3 x_point = glm::dvec3(camera_matrix * glm::vec4(1,0,0,1));
    glm::dvec3 y_point = glm::dvec3(camera_matrix * glm::vec4(0,1,0,1));
    glm::dvec3 fw_vec = glm::normalize(fw_point - settings.projection_origin);
    glm::dvec3 x_vec = glm::normalize(x_point - settings.projection_origin);
    glm::dvec3 y_vec = glm::normalize(y_point - settings.projection_origin);

    //FOV is full vertical FOV
    double tangent = tan( (cam->FOV.Get()/2.0) * 3.14159/180.0 );

    double focus_dist = cam->TraceFocusDistance.Get();

    settings.projection_forward = focus_dist * fw_vec;
    settings.projection_up = focus_dist * tangent * y_vec;
    settings.projection_right = focus_dist * tangent * AspectRatio() * x_vec;

    double aperture_radius = cam->TraceApertureSize.Get() * 0.5;
    settings.aperture_up = aperture_radius * y_vec;
    settings.aperture_right = aperture_radius * x_vec;
    settings.aperture_radius = aperture_radius;

    buffer = new uint8_t[settings.width * settings.height * 3]();

    int num_threads = QThread::idealThreadCount();
    if (num_threads > 1) {
        num_threads -= 1; //leave a free thread so the computer doesn't totally die
    }
    thread_pool.setMaxThreadCount(num_threads);

    if (settings.samplecount_mode!=Camera::TRACESAMPLING_CONSTANT && settings.dynamic_sampling_min_depth==0) {
        int orig = settings.samplecount_mode;
        settings.samplecount_mode = Camera::TRACESAMPLING_CONSTANT;
        settings.constant_samples_per_pixel = 1;

        //TODO make it not hang here
        for (unsigned int i = 0; i < num_threads; i++) {
            thread_pool.start(new RTWorker(*this));
        }
        thread_pool.waitForDone(-1);
        next_render_index.store(0);

        settings.samplecount_mode = orig;
        first_pass_buffer = buffer;
        buffer = new uint8_t[settings.width * settings.height * 3]();
    }

    // Spin off threads
    for (unsigned int i = 0; i < num_threads; i++) {
        thread_pool.start(new RTWorker(*this));
    }
}

RayTracer::~RayTracer() {
    cancelling=true;
    thread_pool.waitForDone(-1);
    delete[] buffer;
    if (first_pass_buffer != nullptr) {
        delete[] first_pass_buffer;
    }
    if (debug_camera_used_) {
        debug_camera_used_->ClearDebugRays();
    }
}

int RayTracer::GetProgress() {
    if (thread_pool.waitForDone(1)) {
        return 100;
    }

    const unsigned int wc = (settings.width+THREAD_CHUNKSIZE-1)/THREAD_CHUNKSIZE;
    const unsigned int hc = (settings.height+THREAD_CHUNKSIZE-1)/THREAD_CHUNKSIZE;
    int complete = (100*next_render_index.fetchAndAddRelaxed(0))/(wc*hc);
    return std::min(complete,99);
}

double RayTracer::AspectRatio() {
    return ((double)settings.width)/((double)settings.height);
}


void RayTracer::ComputePixel(int i, int j, Camera* debug_camera) {
    // Calculate the normalized coordinates [0, 1]
    double x_corner = i * settings.pixel_size_x;
    double y_corner = j * settings.pixel_size_y;

    if (debug_camera) {
        if (debug_camera_used_) {
            debug_camera_used_->ClearDebugRays();
        }
        debug_camera_used_ = debug_camera;
        SampleCamera(x_corner, y_corner, settings.pixel_size_x, settings.pixel_size_y, debug_camera);
        return;
    }

    // Trace the ray!
    glm::vec3 color(0,0,0);

    switch (settings.samplecount_mode) {
        case Camera::TRACESAMPLING_CONSTANT:
            color = SampleCamera(x_corner, y_corner, settings.pixel_size_x, settings.pixel_size_y, debug_camera);
            break;
        default:
            break;
    }

    color = glm::clamp(color, 0.0f, 1.0f);

    // Set the pixel in the render buffer
    uint8_t* pixel = buffer + (i + j * settings.width) * 3;
    pixel[0] = (uint8_t)( 255.0f * color[0]);
    pixel[1] = (uint8_t)( 255.0f * color[1]);
    pixel[2] = (uint8_t)( 255.0f * color[2]);
}


glm::vec3 RayTracer::SampleCamera(double x_corner, double y_corner, double pixel_size_x, double pixel_size_y, Camera* debug_camera)
{
    double x = x_corner + pixel_size_x * 0.5;
    double y = y_corner + pixel_size_y * 0.5;

    glm::dvec3 point_on_focus_plane = settings.projection_origin + settings.projection_forward + (2.0*x-1.0)*settings.projection_right + (2.0*y-1.0)*settings.projection_up;

    glm::vec2 sample = glm::dvec2(0,0);
    double angle = sample.x;
    double radius = sqrt(sample.y);

    glm::dvec3 origin = settings.projection_origin + radius * (sin(angle) * settings.aperture_up + cos(angle) * settings.aperture_right);

    glm::dvec3 dir = glm::normalize(point_on_focus_plane - origin);

    Ray camera_ray(origin, dir);

    return TraceRay(camera_ray, 0, RayType::camera, debug_camera);
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
// Depth is the number of times the ray has intersected an object.
glm::vec3 RayTracer::TraceRay(const Ray& r, int depth, RayType ray_type, Camera* debug_camera)
{
    Intersection i;

    if (debug_camera) {
        glm::dvec3 endpoint = r.at(1000);
        if (trace_scene.Intersect(r, i)) {
            endpoint = r.at(i.t);
            debug_camera->AddDebugRay(endpoint, endpoint+0.25*(glm::dvec3)i.normal, RayType::hit_normal);
        }
        debug_camera->AddDebugRay(r.position, endpoint, ray_type);
    }

    if (trace_scene.Intersect(r, i)) {     
        // TRACE: Implement Raytracing
        // You must implement (see project page for details)
        // 1. Blinn-Phong specular model
        // 2. Light contribution
        // 3. Shadow attenuation
        // 4. Reflection
        // 5. Refraction
        // 6. Anti-Aliasing

        // An intersection occured!  We've got work to do. For now,
        // this code gets the material parameters for the surface
        // that was intersected.
        Material* mat = i.GetMaterial();
        glm::vec3 kd = mat->Diffuse->GetColorUV(i.uv);
        glm::vec3 ks = mat->Specular->GetColorUV(i.uv);
        glm::vec3 ke = mat->Emissive->GetColorUV(i.uv);
        glm::vec3 kt = mat->Transmittence->GetColorUV(i.uv);
        float shininess = mat->Shininess;
        double index_of_refraction = mat->IndexOfRefraction;

        // Interpolated normal
        // Use this to get smooth shading (light direction test, etc)
        glm::vec3 N = i.normal;

        return kd;

        // This is a great place to insert code for recursive ray tracing.
        // Compute the blinn-phong shading, and don't stop there:
        // add in contributions from reflected and refracted rays.

        // To iterate over all light sources in the scene, use code like this:
        // for (auto j = trace_scene.lights.begin(); j != trace_scene.lights.end(); j++) {
        //   TraceLight* trace_light = *j;
        //   Light* scene_light = trace_light->light;
        // }

        // Make sure to test if the Reflections and Refractions checkboxes are enabled in the Render Cam UI
        // Use this condition, only calculate reflection/refraction if enabled:
        // if (settings.reflections) { ... }
        // if (settings.refraction) { ... }

    } else {
        // No intersection. This ray travels to infinity, so we color it according to the background color,
        // which in this (simple) case is just black.
        // EXTRA CREDIT: Use environment mapping to determine the color instead
        glm::vec3 background_color = glm::vec3(0, 0, 0);
        return background_color;
    }
}


// Multi-Threading
RTWorker::RTWorker(RayTracer &tracer_) :
    tracer(tracer_) { }

void RTWorker::run() {
    // Dimensions, in chunks
    const unsigned int wc = (tracer.settings.width+THREAD_CHUNKSIZE-1)/THREAD_CHUNKSIZE;
    const unsigned int hc = (tracer.settings.height+THREAD_CHUNKSIZE-1)/THREAD_CHUNKSIZE;

    unsigned int x, y;
    while (!tracer.cancelling) {
        unsigned int idx = tracer.next_render_index.fetchAndAddRelaxed(1);
        unsigned int x = (idx%wc)*THREAD_CHUNKSIZE;
        unsigned int y = (idx/wc)*THREAD_CHUNKSIZE;
        if (y >= tracer.settings.height) break;
        unsigned int maxX = std::min(x + THREAD_CHUNKSIZE, tracer.settings.width);
        unsigned int maxY = std::min(y + THREAD_CHUNKSIZE, tracer.settings.height);

        for(unsigned int yy = y; yy < maxY && !tracer.cancelling; yy++) {
            for(unsigned int xx = x; xx < maxX && !tracer.cancelling; xx++) {
                tracer.ComputePixel(xx, yy);
            }
        }
    }
}