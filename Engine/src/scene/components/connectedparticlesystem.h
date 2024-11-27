/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CONNECTEDPARTICLESYSTEM_H
#define CONNECTEDPARTICLESYSTEM_H

#include <scene/components/particlesystem.h>

  // EXTRA CREDIT: Implement spring force between connected particles

class ConnectedParticleSystem : public Component {
public:
    ResourceProperty<Material> ParticleMaterial;
    BooleanProperty ParticleVisible;
    DoubleProperty Mass;
    DoubleProperty SpringCoeff;
    DoubleProperty DampCoeff;
    DoubleProperty InitialDisplacement;
    Vec3Property ConstantF;

    ConnectedParticleSystem();

    void UpdateModelMatrix(glm::mat4 model_matrix);
    void InitParticles();
    std::vector<Particle*> GetParticles();
    void StartSimulation();
    void UpdateSimulation(float delta_t, const std::vector<std::pair<SceneObject*, glm::mat4>>& colliders);
    void StopSimulation();
    void ResetSimulation();
    bool IsSimulating();
    bool ShowParticles() { return ParticleVisible.Get(); }

protected:
    glm::mat4 model_matrix_;
    bool simulating_;
    std::vector<std::unique_ptr<Particle>> particles_;
    // EXTRA CREDIT:
    // Add a collection of spring forces, and some way to store which particles are connected.
    static const unsigned int NUM_PARTICLES = 8;
};


#endif // CONNECTEDPARTICLESYSTEM_H