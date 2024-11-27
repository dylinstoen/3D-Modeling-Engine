/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCENECAMERA_H
#define SCENECAMERA_H

#include <animator.h>
#include <components.h>

class SceneCamera {
public:
    SceneCamera() { }
    SceneCamera(const SceneCamera& camera) :
        camera_(camera.camera_.FOV.Get(),
                camera.camera_.RenderWidth.Get(),
                camera.camera_.RenderHeight.Get(),
                camera.camera_.NearPlane.Get(),
                camera.camera_.FarPlane.Get(),
                camera.camera_.Width.Get()
        ),
        screen_size_(camera.screen_size_)
    {
        if (camera.camera_.IsPerspective.Get()) ToPerspective();
        else ToOrthographic();
        transform_.CopyFrom(camera.transform_);
        anchor_.CopyFrom(camera.anchor_);
    }

    glm::mat4 GetTransform() const;
    // View Matrix is the inverse of the Camera's transforms
    glm::mat4 GetView() const {
        return glm::inverse(GetTransform());
    }
    glm::mat4 GetProjection() const { return camera_.GetProjection(); }
    glm::vec2 GetScreenSize() const { return screen_size_; }
    double GetFOV() const { return camera_.FOV.Get(); }

    glm::vec3 GetAnchorPosition() { return anchor_.Translation.Get(); }

    void GetRay(glm::vec2 screen_coords, glm::vec3& p, glm::vec3& v) const;

    // Focus the camera on the position (has nothing to do with focal length)
    // TODO: Be more intelligent about always keeping the object in view
    void SetFocus(const glm::vec3& position);
    void SetRotation(glm::vec3 axis) { anchor_.Rotation.Set(axis); }
    glm::vec3 GetRotation() { return anchor_.Rotation.Get(); }
    void Orbit(glm::vec3 axis, double angle);
    void EulerRotate(float yaw, float pitch);
    void Zoom(float distance);
    void ZoomRelative(float distance); // scale zoom distance by current zoom
    void Move(glm::vec3 translation, Space space=Space::Local);
    // TODO: I don't think Space::World is working right now. The translation specified is in world, but say it goes -1.0 Y
    // If the object is rotated, it would move -1.0Y along its own axis.

    // TODO: Moves the camera around in 2D relative to its up vector
    // void Pan(float dx, float dy);

    double GetWidth() const { return camera_.Width.Get(); }
    double GetZDistance() { return transform_.Translation.Get().z + anchor_.Translation.Get().z; }
    double GetDistance(glm::vec3 p) const;

    // Camera Properties
    void ToOrthographic() { camera_.IsPerspective.Set(false); }
    void ToPerspective() { camera_.IsPerspective.Set(true); }
    bool IsPerspective() const { return camera_.IsPerspective.Get(); }
    void SetWidth(double width) { camera_.Width.Set(width); }
    void SetFOV(double fov) { camera_.FOV.Set(fov); }
    void SetNearPlane(double near_plane) { camera_.NearPlane.Set(near_plane); }
    void SetFarPlane(double far_plane) { camera_.FarPlane.Set(far_plane); }
    void SetScreenSize(unsigned int width, unsigned int height) {
        assert(height > 0);
        screen_size_ = glm::vec2(width, height);
        camera_.RenderWidth.Set(width);
        camera_.RenderHeight.Set(height);
        camera_.Width.Set(2*GetZDistance()*tan(glm::radians(camera_.FOV.Get()/2))*camera_.GetAspectRatio());
    }
private:
    Camera camera_;
    glm::vec2 screen_size_;
    // The Camera transform is only ever moved along the Z-Axis according to the zoom.
    // Any other transformations should be performed on the Anchor, which can be considered the Camera's parent.
    Transform transform_;
    Transform anchor_;
};

#endif // SCENECAMERA_H
