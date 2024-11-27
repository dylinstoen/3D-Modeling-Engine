/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "connectedparticlesystem.h"
#include "spherecollider.h"
#include "planecollider.h"
#include "cylindercollider.h"
#include <scene/sceneobject.h>

REGISTER_COMPONENT(ConnectedParticleSystem, ConnectedParticleSystem)

ConnectedParticleSystem::ConnectedParticleSystem() :
    ParticleMaterial(AssetType::Material),
    ParticleVisible(true),
    Mass(1.0f, 0.0f, 10.0f, 0.1f),
    SpringCoeff(50.0f, 0.0f, 100.0f, 0.1f),
    DampCoeff(2.0f, 0.0f, 100.0f, 1.0f),
    ConstantF(glm::vec3(0.0f, -9.8f, 0.0f)),
    InitialDisplacement(1.1f, 0.5f, 2.0f, 0.1f),
    simulating_(false)
{
    AddProperty("Material", &ParticleMaterial);
    AddProperty("Show Particles", &ParticleVisible);
    AddProperty("Mass", &Mass);
    AddProperty("Spring Constant", &SpringCoeff);
    AddProperty("Damping Constant", &DampCoeff);
    AddProperty("Constant Force", &ConstantF);
    AddProperty("Initial Displacement", &InitialDisplacement);
}

void ConnectedParticleSystem::UpdateModelMatrix(glm::mat4 model_matrix) {
   model_matrix_ = model_matrix;
}

void ConnectedParticleSystem::InitParticles() {
    if (!simulating_) return;

    // EXTRA CREDIT: Create some particles!
}

std::vector<Particle*> ConnectedParticleSystem::GetParticles() {
    // Return a vector of particles (used by renderer to draw them)
    std::vector<Particle*> particles;
    for (auto& particle : particles_) particles.push_back(particle.get());
    return particles;
}

void ConnectedParticleSystem::StartSimulation() {
    simulating_ = true;
    // EXTRA CREDIT: Set spring force parameters
    ResetSimulation();
}

void ConnectedParticleSystem::UpdateSimulation(float delta_t, const std::vector<std::pair<SceneObject*, glm::mat4>>& colliders) {
    if (!simulating_) return;

    // EXTRA CREDIT: Simulate connected particle system
}

void ConnectedParticleSystem::StopSimulation() {
    simulating_ = false;
}

void ConnectedParticleSystem::ResetSimulation() {
    // EXTRA CREDIT: Reset your particle system, particle state, etc.
    InitParticles();
}

bool ConnectedParticleSystem::IsSimulating() {
    return simulating_;
}