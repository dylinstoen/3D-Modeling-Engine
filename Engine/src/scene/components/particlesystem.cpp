/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "particlesystem.h"
#include "spherecollider.h"
#include "planecollider.h"
#include "cylindercollider.h"
#include <scene/sceneobject.h>

REGISTER_COMPONENT(ParticleSystem, ParticleSystem)

ParticleSystem::ParticleSystem() :
    ParticleGeometry({"Sphere"}, 0),
    ParticleMaterial(AssetType::Material),
    Mass(0.1f, 0.0f, 10.0f, 0.1f),
    Period(0.5f, 0.0f, 1.0f, 0.01f),
    InitialVelocity(glm::vec3(5.0f, 5.0f, 0.0f)),
    ConstantF(glm::vec3(0.0f, -9.8f, 0.0f)),
    DragF(0.0f, 0.0f, 1.0f, 0.01f),
    constant_force_(ConstantF.Get()),
    drag_force_(DragF.Get()),
    // REQUIREMENT:
    // init drag force with DragF -- refer to how we deal with constant_force_
    // remember  (f = -k_d * v), where DragF represents k_d
    num_particles_(0),
    particle_index_(0),
    simulating_(false)
{
    AddProperty("Geometry", &ParticleGeometry);
    AddProperty("Material", &ParticleMaterial);
    AddProperty("Mass", &Mass);
    AddProperty("Period (s)", &Period);
    AddProperty("Initial Velocity", &InitialVelocity);
    AddProperty("Constant Force", &ConstantF);
    AddProperty("Drag Coefficient", &DragF);

    ParticleGeometry.ValueSet.Connect(this, &ParticleSystem::OnGeometrySet);

    forces_.push_back(std::shared_ptr<Force>(&constant_force_));
    forces_.push_back(std::shared_ptr<Force>(&drag_force_));

    // REQUIREMENT: 
    //    add viscous drag force into forces_ array if your drag force also inherit from class Force
    //    If not, you could use your own way to prepare your drag force
}

void ParticleSystem::UpdateModelMatrix(glm::mat4 model_matrix) {
   model_matrix_ = model_matrix;
}

void ParticleSystem::EmitParticles() {
    if (!simulating_) return;

    // REQUIREMENT:
    // Create some particles!
    //    - We have designed a class Particle for you
    //    - We've provided some UI controls for you
    //          -- Mass.Get() defines particle mass, and
    //          -- InitialVelocity.Get() defines particle init velocity in local object space
    //    - Notice particles should be created in world space. (use model_matrix_ to transform particles from local object space to world space)
    // Store particles in the member variable particles_
    // For performance reasons, limit the amount of particles that exist at the same time
    // to some finite amount (MAX_PARTICLES). Either delete or recycle old particles as needed.
    glm::vec3 init_position = model_matrix_ * glm::vec4(glm::vec3(0,0,0), 1.0);
    glm::vec3 init_velocity = glm::mat3(model_matrix_) * InitialVelocity.Get();
    glm::vec3 init_rotation = model_matrix_ * glm::vec4(glm::vec3(0,0,0), 1.0);
    particles_.push_back(std::make_unique<Particle>(Mass.Get(), init_position, init_velocity, init_rotation));
    num_particles_ += 1;
    if (num_particles_ > MAX_PARTICLES) {
        particles_.erase(particles_.begin());
        num_particles_ -= 1;
    }
    // Reset the time
    time_to_emit_ = Period.Get();
}

std::vector<Particle*> ParticleSystem::GetParticles() {
    // Return a vector of particles (used by renderer to draw them)
    std::vector<Particle*> particles;
    for (auto& particle : particles_) particles.push_back(particle.get());
    return particles;
}

void ParticleSystem::StartSimulation() {
    simulating_ = true;
    constant_force_.SetForce(ConstantF.Get());
    // REQUIREMENT:
    // Set your added drag force as DragF.Get() -- Refer to what we did on constact_force_
    drag_force_.SetDrag(DragF.Get());
    ResetSimulation();
}

void ParticleSystem::UpdateSimulation(float delta_t, const std::vector<std::pair<SceneObject*, glm::mat4>>& colliders) {
    if (!simulating_) return;

    // Emit Particles
    time_to_emit_ -= delta_t;
    if (time_to_emit_ <= 0.0) EmitParticles();

    // REQUIREMENT:
    // For each particle ...
    //      Calculate forces
    //      Solve the system of forces using Euler's method
    //      Update the particle
    //      Check for and handle collisions
   for (auto& p : particles_)  {
       glm::vec3 force = constant_force_.GetForce(*p) + drag_force_.GetForce(*p);
       p->Position = p->Position + delta_t * p->Velocity;
       p->Velocity = p->Velocity + delta_t * force / p->Mass;
   }
//    if (num_particles_ == 0) {
//        return;
//    }
//    Particle *front_p = particles_.front().get();
//    glm::vec3 force =constant_force_.GetForce(*front_p) + drag_force_.GetForce(*front_p);
//    auto prev_p = front_p->Position + delta_t * front_p->Velocity;
//    auto prev_v = front_p->Velocity + delta_t * force / front_p->Mass;
//    for (auto& p : particles_)  {
//        auto temp = p->Position;
//        p->Position = prev_p;
//        prev_p = temp;
//        temp = p->Velocity;
//        p->Velocity = prev_v;
//        prev_v = temp;
//    }

    // Collision code might look something like this:
    for (auto& kv : colliders) {

        SceneObject* collider_object = kv.first;
        glm::mat4 collider_model_matrix = kv.second;
        glm::mat4 collider_model_matrix_inverse = glm::inverse(collider_model_matrix);

        static const double EPSILON = 0.1;
        float particle_radius = 0.5f;

        // When checking collisions, remember to bring particles from world space to collider local object space
        // The trasformation matrix can be derived by taking invese of collider_model_matrix
        if (SphereCollider* sphere_collider = collider_object->GetComponent<SphereCollider>()) {
            // Check for Sphere Collision
            for (auto& p : particles_)  {
                glm::vec3 local_p_pos = collider_model_matrix_inverse * glm::vec4(p->Position, 1.0);
                glm::vec3 local_p_v = glm::mat3(collider_model_matrix_inverse) * p->Velocity;
                double collide_dist = sphere_collider->Radius.Get() + particle_radius + EPSILON;
                glm::vec3 N = glm::normalize(local_p_pos);
                if (glm::length2(local_p_pos) <= collide_dist * collide_dist &&
                    glm::dot(local_p_v, N) < 0.0) {
                    glm::vec3 Vn = glm::dot(N, local_p_v) * N;
                    glm::vec3 Vt = local_p_v - Vn;
                    p->Velocity = glm::mat3(collider_model_matrix) * (Vt - float(sphere_collider->Restitution.Get()) * Vn);
                }
            }
        } else if (PlaneCollider* plane_collider = collider_object->GetComponent<PlaneCollider>()) {
            // Check for Plane Collision
            for (auto& p : particles_)  {
                glm::vec3 local_p_pos = collider_model_matrix_inverse * glm::vec4(p->Position, 1.0);
                glm::vec3 local_p_v = glm::mat3(collider_model_matrix_inverse) * p->Velocity;
                glm::vec3 N = glm::vec3(0,0,1);
                double collide_dist = particle_radius + EPSILON;
                // width is x
                glm::vec3 Nx = glm::vec3(1,0,0);
                glm::vec3 Ny = glm::vec3(0,1,0);
                double half_width = plane_collider->Width.Get() / 2;
                double half_height = plane_collider->Width.Get() / 2;
                if (glm::length2(local_p_pos * Nx) <= half_width * half_width &&
                    glm::length2(local_p_pos * Ny) <= half_height * half_height &&
                    glm::length2(local_p_pos * N) <= collide_dist * collide_dist &&
                    glm::dot(local_p_v, N) < 0.0) {
                    glm::vec3 Vn = glm::dot(N, local_p_v) * N;
                    glm::vec3 Vt = local_p_v - Vn;
                    float t = float(plane_collider->Restitution.Get());
                    glm::vec3 test = (Vt - t * Vn);
                    p->Velocity = glm::mat3(collider_model_matrix) * (Vt - t * Vn);
                }
            }
        }
        // When updating particle velocity, remember it's in the worls space.
    }
}

void ParticleSystem::StopSimulation() {
    simulating_ = false;
}

void ParticleSystem::ResetSimulation() {
    // Clear all particles
    time_to_emit_ = Period.Get();
    particles_.clear();
    num_particles_ = 0;
}

bool ParticleSystem::IsSimulating() {
    return simulating_;
}


void ParticleSystem::OnGeometrySet(int c) {
    GeomChanged.Emit(ParticleGeometry.GetChoices()[c]);
}
