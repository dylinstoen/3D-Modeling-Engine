/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <signal.h>
#include <scene/components/component.h>
#include <resource/material.h>


struct Particle {
public:
    float Mass;
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec3 Rotation;

    Particle(float mass_ = 1.0f, glm::vec3 position_ = glm::vec3(0,0,0), glm::vec3 velocity_ = glm::vec3(0,0,0), glm::vec3 rotation_ = glm::vec3(0,0,0)) :
        Mass(mass_), Position(position_), Velocity(velocity_), Rotation(rotation_) { }
};

class Force {
public:
    virtual glm::vec3 GetForce(Particle& p) const = 0;
};

class ConstantForce : public Force {
public:
    ConstantForce(glm::vec3 force) : force_(force) { }
    void SetForce(glm::vec3 f) { force_ = f; }
    virtual glm::vec3 GetForce(Particle &p) const override {
        return p.Mass * force_;
    }
private:
    glm::vec3 force_;
};

  // REQUIREMENT:
  // Add viscous drag force (f = -k_d * v)
  // You might want to refer to class ConstantForce as template, and create an new class DragForce
  // Note this is optional. You could design your own way to represent DragForce, for sure.

class DragForce : public Force {
public:
    DragForce(double kd) : kd_(kd) { }
    void SetDrag(double kd) { kd_ = kd; }
    virtual glm::vec3 GetForce(Particle &p) const override {
        return float(- kd_) * p.Velocity;
    }
private:
    double kd_;
};

class ParticleSystem : public Component {
public:
    ChoiceProperty ParticleGeometry;
    ResourceProperty<Material> ParticleMaterial;
    DoubleProperty ParticleScale;
    DoubleProperty Mass;
    DoubleProperty Period;
    Vec3Property InitialVelocity;
    Vec3Property ConstantF;
    DoubleProperty DragF;   // Use this for k_d in viscous drag force

    // EXTRA CREDIT: Allow the user to enable billboards. See glRenderer::Render(SceneObject&, ParticleSystem).
    // BooleanProperty Billboards;

    ParticleSystem();

    void UpdateModelMatrix(glm::mat4 model_matrix);
    void EmitParticles();
    std::vector<Particle*> GetParticles();
    void StartSimulation();
    void UpdateSimulation(float delta_t, const std::vector<std::pair<SceneObject*, glm::mat4>>& colliders);
    void StopSimulation();
    void ResetSimulation();
    bool IsSimulating();

    Signal1<std::string> GeomChanged;

protected:
    static const unsigned int MAX_PARTICLES = 100;
    unsigned int num_particles_;
    unsigned int particle_index_;
    ConstantForce constant_force_;
    // REQUIREMENT:
    // Add a drag force member
    DragForce drag_force_;
    glm::mat4 model_matrix_;
    double time_to_emit_;
    bool simulating_;
    std::vector<std::unique_ptr<Particle>> particles_;
    std::vector<std::shared_ptr<Force>> forces_;

    void OnGeometrySet(int);
};


#endif // PARTICLESYSTEM_H
