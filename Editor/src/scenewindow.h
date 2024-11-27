/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCENEWINDOW_H
#define SCENEWINDOW_H

#include <glinclude.h>
#include <QOpenGLWidget>
#include <animator.h>
#include <scene/scene.h>
#include <scene/trackball.h>
#include <scene/translator.h>
#include <scene/renderer.h>
#include <fpscounter.h>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QTimer>
#include <widgets/menutoolbutton.h>
#include <gizmomanager.h>
#include <opengl/glrenderer.h>

class CameraMenuItem;

class SceneWindow : public QOpenGLWidget {
    Q_OBJECT

public:
    // TODO: Get rid of oldstyle enum
    enum MouseMode {
        MOUSEMODE_CAMERA,
        MOUSEMODE_TRANSLATE,
        MOUSEMODE_ROTATE,
        MOUSEMODE_SCALE,
        NUM_MOUSEMODES
    };

    SceneWindow(QVBoxLayout& managing_layout, QWidget *parent = nullptr);

    void ShowNormals(bool show);
    void ShowSelected(bool show);
    // Requests a redraw of the scene
    void Redraw();
    // Focuses the scene on the specified location
    void SetFocus(glm::vec3 world_position);
    void SetScene(Scene& scene, SceneCamera* camera=nullptr);
    void SetMode(MouseMode mode) { mode_ = mode; Redraw(); }
    void SetVertexEditing(bool edit);
    MouseMode GetMode() const { return mode_; }
    const SceneCamera* GetSceneCamera() const { return scene_camera_; }

    void SetRenderCam(SceneObject* render_cam);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void InitializeFrameBuffers();

    SceneObject* FindVertexController();
    void RemoveVertexController();

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    std::unique_ptr<GLRenderer> renderer_;
    Scene* scene_;
    SceneCamera* scene_camera_;
    SceneObject* render_cam_;
    Trackball trackball_;
    Translator translator_;
    uint64_t selected_object_;
    FPSCounter fps_counter_;
    GizmoManager* gizmo_manager_;

    // UI Elements
    MenuToolButton* camera_tool;
    QAction* ortho_x_action;
    QAction* ortho_y_action;
    QAction* ortho_z_action;

    // Render cam menu management
    QMenu* render_cam_menu;
    QAction* perspective_action;
    QAction* render_cam_action;
    std::unordered_map<const SceneObject*, CameraMenuItem*> cam2action;
    std::unordered_map<const CameraMenuItem*, SceneObject*> action2cam;

    bool needs_update_;

    // Camera Focusing
    glm::vec3 focus_end_;
    glm::vec3 focus_start_;
    double focus_time_;
    QTimer* timer_;

    const float rotation_speed_ = 2.0f;
    const float euler_rotation_speed_ = 0.15f;
    const float translation_speed_ = 0.01f;
    const float zoom_speed_ = 0.0005f;
    QTimer* camera_timer_;
    std::set<int> keys_down_;
    glm::vec3 camera_velocity_;

    MouseMode mode_;
    bool vertex_editing_;
    Space manipulation_space_;
    bool mouse_moved_, manipulating_camera_;
    bool clicked_object_;
    bool render_normals_;
    bool render_selected_;
    bool freeze_controls_;

    // Read from framebuffer targets
    GLuint framebuffer_tex_, framebuffer_depth_, fbo_;
    GLuint render_framebuffer_tex_, render_framebuffer_depth_, render_fbo_;
    float* buffer_image_;
    unsigned char* saved_image_;
    int bufsize;

    // Trackball stuff
    glm::vec3 start_;
    glm::vec3 end_;

    int lastmousex, lastmousey;

signals:
    void ObjectSelected(uint64_t uid);
    void ObjectTranslated(int x, int y);
    void ObjectTranslationBegin(
            const SceneCamera* scene_camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void ObjectRotated(int x, int y);
    void ObjectRotationBegin(
            const SceneCamera* scene_camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void ObjectScaled(int x, int y);
    void ObjectScaleBegin(
            const SceneCamera* scene_camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void TearoffRequested(SceneCamera* camera);
    void AnimationRequested();

public slots:
    void OnObjectSelected(uint64_t selected_object);
    void SetMouseMode(MouseMode mousemode) { mode_ = mousemode; }
    void SetManipulationSpace(Space space) { manipulation_space_ = space; }
    void SetOrthoX();
    void SetOrthoY();
    void SetOrthoZ();
    void ContextChanged(bool);

    void OnCameraAdded(SceneObject& camera);
    void OnCameraRemoved(SceneObject& camera);
};

class CameraMenuItem : public QObject {
    Q_OBJECT
    public:
        CameraMenuItem(SceneObject& object, SceneWindow* parent) :
            parent_(parent),
            camera_(&object)
        {
            action = new QAction(QString::fromStdString(object.GetName()), parent_);
            connect(action, &QAction::triggered, this, [&, this](){
                parent_->SetRenderCam(camera_);
            });
            camera_->NameChanged.Connect(this, &CameraMenuItem::NameChanged);
        }
        ~CameraMenuItem() { delete action; }
        void NameChanged(std::string s) {
            action->setText(QString::fromStdString(s));
        }

        SceneWindow* parent_;
        SceneObject* camera_;
        QAction* action;
};

#endif // SCENEWINDOW_H
