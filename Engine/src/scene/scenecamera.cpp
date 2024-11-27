/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "scenecamera.h"
#include <algorithm>

glm::mat4 SceneCamera::GetTransform() const {
    if (camera_.IsPerspective.Get()) {
        return anchor_.GetMatrix() * transform_.GetMatrix();
    } else {
        return anchor_.GetMatrix();
    }
}

void SceneCamera::GetRay(glm::vec2 screen_coords, glm::vec3& p, glm::vec3& v) const {
    glm::vec2 p0 = screen_coords - (glm::vec2(0.5,0.5)*screen_size_) + glm::vec2(0.5,0.5);
    if (camera_.IsPerspective.Get()) {
        double focal_length = screen_size_.y/(2*tan(camera_.FOV.Get()*M_PI/360.f));
        glm::vec3 v0(p0.xy(), -focal_length);
        p = GetTransform()*glm::vec4(0,0,0,1);
        v = GetTransform()*glm::vec4(normalize(v0),0);
    } else {
        double w = camera_.Width.Get();
        p0 *= glm::vec2(w/screen_size_.x, w/screen_size_.x);
        p = GetTransform()*glm::vec4(p0.xy(), 0, 1);
        v = GetTransform()*glm::vec4(0,0,-1,0);
    }
    v = normalize(v);
}

// TODO: Be more intelligent about always keeping the object in view
void SceneCamera::SetFocus(const glm::vec3& position) {
    transform_.Translation.Set(glm::vec3(0, 0, 2.5f));
    anchor_.Translation.Set(position);
    camera_.Width.Set(2*GetZDistance()*tan(glm::radians(camera_.FOV.Get()/2))*camera_.GetAspectRatio());
}

void SceneCamera::EulerRotate(float yaw, float pitch)
{
    if (camera_.IsPerspective.Get()) {
        glm::vec3 fwd = anchor_.GetForward();

        pitch = std::min(std::max(pitch + glm::degrees(asinf(-fwd.y)), -89.9f), 89.9f);
        yaw = fmodf(yaw + glm::degrees(atan2f(fwd.x,fwd.z)), 360.0f);

        glm::vec3 curang = anchor_.Rotation.Get();

        anchor_.Rotation.Set(glm::vec3(0, 0, 0));
        //The rotation seems to be applied before current angle?
        anchor_.Rotate(glm::vec3(0,1,0), yaw);
        anchor_.Rotate(glm::vec3(1,0,0), pitch);
    }
}

void SceneCamera::Orbit(glm::vec3 axis, double angle) {
    if (camera_.IsPerspective.Get()) {
        anchor_.Rotate(axis, angle);
    }
}

void SceneCamera::Zoom(float distance) {
    transform_.Translate(glm::vec3(0, 0, distance), Space::Local);
    if (transform_.Translation.Get().z < 0.01f) {
        transform_.Translation.Set(glm::vec3(0, 0, 0.01f));
    }
    double z = GetZDistance();
    SetWidth(camera_.Width.Get()*z/(z-distance));
}

void SceneCamera::ZoomRelative(float distance) {
    double scale = transform_.Translation.Get().z;
    Zoom(scale * distance);
}

void SceneCamera::Move(glm::vec3 translation, Space space) {
    anchor_.Translate(translation, space);
}

double SceneCamera::GetDistance(glm::vec3 p) const {
    glm::vec3 pos = GetView()*glm::vec4(p,1);
    return std::abs(pos.z);
}
