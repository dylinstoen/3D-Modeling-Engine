/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "tracescene.h"

#include <trace/ray.h>

// Multi-Threading
#define THREAD_CHUNKSIZE 16
#include <QThreadPool>
#include <QRunnable>
#include <QAtomicInt>
#include "randomsampler.h"

class Scene;
class Camera;

enum RayType {
  camera,
  reflection,
  diffuse_reflection,
  refraction,
  shadow,
  hit_normal //actually just a vector not a ray
};

class RayTracer {
  public:
    struct RayTracerSettings {
        unsigned int width;
        unsigned int height;
        double pixel_size_x;
        double pixel_size_y;


        int random_mode;
        bool diffuse_reflection;
        bool caustics;
        bool random_branching;

        int samplecount_mode;
        unsigned int constant_samples_per_pixel;
        unsigned int dynamic_sampling_min_depth;
        unsigned int dynamic_sampling_max_depth;
        double adaptive_max_diff_squared;
        double max_stderr;

        unsigned int max_depth; // Maximum depth of recursion
        bool shadows;
        bool translucent_shadows;
        bool reflections;
        bool refractions;
        bool fresnel;
        bool beers;


        glm::dvec3 projection_origin;
        glm::dvec3 projection_forward; //length = focus distance
        glm::dvec3 projection_up; //from center of focus plane to edge
        glm::dvec3 projection_right;

        glm::dvec3 aperture_up;
        glm::dvec3 aperture_right;
        float aperture_radius;
    };
    
    RayTracer(Scene& scene, SceneObject& camera);
    ~RayTracer();

    int GetProgress();

    double AspectRatio();

    // computes+colors the pixel at this window coordinate
    void ComputePixel(int i, int j, Camera* debug_camera=nullptr);


    std::string GetErrorMessage() {
        std::string ret = errormsg_;
        errormsg_ = "";
        return ret;
    }

    RayTracerSettings settings;
    uint8_t* buffer;
    uint8_t* first_pass_buffer;

    QAtomicInt next_render_index;
    bool cancelling;

private:
    int second_pass_sampling_mode;
    QThreadPool thread_pool;
    TraceScene trace_scene;
    std::string errormsg_;
    Camera* debug_camera_used_ = nullptr;

    glm::vec3 FirstPassColor(int x, int y) {
        glm::vec3 c(first_pass_buffer[3*(x + y*settings.width)], first_pass_buffer[3*(x + y*settings.width)+1], first_pass_buffer[3*(x + y*settings.width)+2]);
        return c/255.0f;
    }

    // Recursively traces ray through the scene. Depth is used to end recursion.
    // Thresh is used to terminate ray tracing early if the ray contribution is too little.
    glm::vec3 TraceRay(const Ray& r, int depth, RayType ray_type, Camera* debug_camera=nullptr);
    glm::vec3 SampleCamera(double x_corner, double y_corner, double pixel_size_x, double pixel_size_y, Camera* debug_camera=nullptr);
};

// Worker thread for raytracing
class RTWorker : public QObject, public QRunnable {
    Q_OBJECT
  public:
    RTWorker(RayTracer& tracer_);
    virtual void run() override;
  private:
    RayTracer& tracer;
};

#endif // RAYTRACER_H
