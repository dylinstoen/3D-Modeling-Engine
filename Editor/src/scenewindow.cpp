/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <glextinclude.h>
#include <QOpenGLWidget>
#include <scenewindow.h>
#include <scene/components/camera.h>
#include <opengl/glrenderer.h>
#include <QMouseEvent>
#include <QSurface>
#include <QVBoxLayout>
#include <QToolBar>
#include <widgets/menutoolbutton.h>
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <widgets/filepicker.h>

std::map<int, glm::vec3> cameraControls = {
    { Qt::Key_Right, glm::vec3(1,0,0) },
    { Qt::Key_Left, glm::vec3(-1,0,0) },
    { Qt::Key_PageUp, glm::vec3(0,1,0) },
    { Qt::Key_PageDown, glm::vec3(0,-1,0) },
    { Qt::Key_Down, glm::vec3(0,0,1) },
    { Qt::Key_Up, glm::vec3(0,0,-1) }
};

bool IsVertexController(SceneObject* o) {
    if (o==nullptr) return false;
    return o->IsInternal() && o->GetName()=="vertex";
}

SceneWindow::SceneWindow(QVBoxLayout& managing_layout, QWidget *parent) :
    QOpenGLWidget(parent),
    renderer_(std::make_unique<GLRenderer>()),
    scene_(nullptr),
    scene_camera_(nullptr),
    render_cam_(nullptr),
    selected_object_(0),
    gizmo_manager_(nullptr),
    perspective_action(nullptr),
    timer_(new QTimer(this)),
    camera_timer_(new QTimer(this)),
    camera_velocity_(0,0,0),
    needs_update_(true),
    mode_(MOUSEMODE_CAMERA),
    vertex_editing_(false),
    manipulation_space_(Space::Local),
    mouse_moved_(false),
    manipulating_camera_(false),
    clicked_object_(false),
    render_normals_(false),
    render_selected_(true),
    freeze_controls_(false),
    buffer_image_(nullptr),
    saved_image_(nullptr),
    bufsize(0),
    lastmousex(0),
    lastmousey(0)
{    
    setFocusPolicy(Qt::ClickFocus);

    // Precise timers try to keep millisecond accuracy
    timer_->setTimerType(Qt::PreciseTimer);

    // Shading Menu
    QAction* shaded_action = new QAction(tr("Shaded"), this);
    connect(shaded_action, &QAction::triggered, this, [this](bool) {
        renderer_->SetRenderingMode(RenderingMode::Shaded);
        Redraw();
    });

    QAction* wireframe_action = new QAction(tr("Wireframe"), this);
    connect(wireframe_action, &QAction::triggered, this, [this](bool) {
        renderer_->SetRenderingMode(RenderingMode::Wireframe);
        Redraw();
    });

    QAction* points_action = new QAction(tr("Points"), this);
    connect(points_action, &QAction::triggered, this, [this](bool) {
        renderer_->SetRenderingMode(RenderingMode::Points);
        Redraw();
    });

    QActionGroup* shading_group = new QActionGroup(this);
    shading_group->setExclusive(true);
    shading_group->addAction(shaded_action);
    shading_group->addAction(wireframe_action);
    shading_group->addAction(points_action);

    QMenu* shading_menu = new QMenu(tr("Shading"), this);
    shading_menu->addAction(shaded_action);
    shading_menu->addAction(wireframe_action);
    shading_menu->addAction(points_action);

    // Display Gizmos Menu
    QAction* lights_action = new QAction(tr("Lights"), this);
    lights_action->setCheckable(true);
    lights_action->setChecked(true);
    connect(lights_action, &QAction::triggered, this, [this](bool checked) { renderer_->DisplayLights(checked); Redraw(); });

    QAction* camera_action = new QAction(tr("Camera"), this);
    camera_action->setCheckable(true);
    camera_action->setChecked(true);
    connect(camera_action, &QAction::triggered, this, [this](bool checked) { renderer_->DisplayCamera(checked); Redraw(); });

    QAction* colliders_action = new QAction(tr("Colliders"), this);
    colliders_action->setCheckable(true);
    colliders_action->setChecked(true);
    connect(colliders_action, &QAction::triggered, this, [this](bool checked) { renderer_->DisplayColliders(checked); Redraw(); });

    QMenu* gizmos_menu = new QMenu(tr("Display"), this);
    QAction* display_action = new QAction(tr("Display"), this);
    gizmos_menu->addAction(lights_action);
    gizmos_menu->addAction(camera_action);
    gizmos_menu->addAction(colliders_action);

    // Camera Menu
    render_cam_menu = new QMenu(tr("Render Camera"), this);
    perspective_action = new QAction(tr("Perspective"), this);
    connect(perspective_action, &QAction::triggered, this, [this]() {
        render_cam_ = nullptr;
        scene_camera_->ToPerspective();
        Redraw();
    });
    ortho_x_action = new QAction(tr("Orthographic X"), this);
    ortho_y_action = new QAction(tr("Orthographic Y"), this);
    ortho_z_action = new QAction(tr("Orthographic Z"), this);
    connect(ortho_x_action, SIGNAL(triggered(bool)), this, SLOT(SetOrthoX()));
    connect(ortho_y_action, SIGNAL(triggered(bool)), this, SLOT(SetOrthoY()));
    connect(ortho_z_action, SIGNAL(triggered(bool)), this, SLOT(SetOrthoZ()));

    // TODO: Change this to be a MainWindow action
    QAction* tearoff_action = new QAction(tr("New View"), this);
    connect(tearoff_action, &QAction::triggered, this, [this]() {
        emit TearoffRequested(scene_camera_);
    });

    QAction* cam_from_current_action = new QAction(tr("Create Render Cam"), this);
    cam_from_current_action->setToolTip(tr("Create Render Cam from Current View"));
    connect(cam_from_current_action, &QAction::triggered, this, [this]() {
        if (scene_) {
            static int num_nodes_created = 0;
            num_nodes_created++;
            SceneObject& cam = scene_->CreateCamera("Render Camera " + std::to_string(num_nodes_created));
            cam.GetTransform().SetFromMatrix(scene_camera_->GetTransform());
            Camera* camera = cam.GetComponent<Camera>();
            glm::vec2 sz = scene_camera_->GetScreenSize();
            camera->RenderWidth.Set(sz.x);
            camera->RenderHeight.Set(sz.y);
            camera->IsPerspective.Set(scene_camera_->IsPerspective());
            camera->FOV.Set(scene_camera_->GetFOV());
        }
    });

    QMenu* camera_menu = new QMenu(tr("Camera"), this);
    camera_menu->addAction(ortho_x_action);
    camera_menu->addAction(ortho_y_action);
    camera_menu->addAction(ortho_z_action);
    camera_menu->addAction(perspective_action);
    camera_menu->addMenu(render_cam_menu);

    MenuToolButton* shading_tool = new MenuToolButton();
    shading_tool->setMenu(shading_menu);
    shading_tool->setDefaultAction(shaded_action);

    QToolButton* gizmos_tool = new QToolButton();
    gizmos_tool->setDefaultAction(display_action);
    gizmos_tool->setMenu(gizmos_menu);
    gizmos_tool->setPopupMode(QToolButton::InstantPopup);

    camera_tool = new MenuToolButton();
    camera_tool->setMenu(camera_menu);
    camera_tool->setDefaultAction(perspective_action);

    QToolButton* tearoff_tool = new QToolButton();
    tearoff_tool->setDefaultAction(tearoff_action);

    QToolButton* rendercam_tool = new QToolButton();
    rendercam_tool->setDefaultAction(cam_from_current_action);

    QToolBar* menubar = new QToolBar(this);
    menubar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    menubar->addWidget(shading_tool);
    menubar->addWidget(gizmos_tool);
    menubar->addWidget(camera_tool);
    menubar->addWidget(tearoff_tool);
    menubar->addWidget(rendercam_tool);
    menubar->setStyleSheet("QToolBar{spacing:5px;}");

    // Insert the menubar above the OpenGLWidget
    managing_layout.insertWidget(0, menubar);

    connect(timer_, &QTimer::timeout, this, [this](){
        focus_time_ += 0.1f;
        glm::vec3 focus_pos = glm::lerp<float>(focus_start_, focus_end_, focus_time_);
        scene_camera_->SetFocus(focus_pos);
        if (focus_time_ >= 1.0f) timer_->stop();
        Redraw();
    });

    connect(camera_timer_, &QTimer::timeout, this, [this]() {
        for (auto k : keys_down_) {
            camera_velocity_ += cameraControls.at(k);
        }
        if (glm::length2(camera_velocity_) > 0.001f || needs_update_) {
            scene_camera_->Move(camera_velocity_*0.016f, Space::Local);
            update();
            camera_velocity_ = camera_velocity_*0.7f;
            needs_update_ = false;
        } else {
            camera_velocity_ = glm::vec3(0,0,0);
        }
    });
    camera_timer_->start(16);
}

void SceneWindow::ContextChanged(bool) {
    renderer_->ContextChanged();
}

void SceneWindow::ShowNormals(bool show) {
    render_normals_ = show;
    Redraw();
}

void SceneWindow::ShowSelected(bool show) {
    render_selected_ = show;
    Redraw();
}

void SceneWindow::SetScene(Scene& scene, SceneCamera* camera) {
    scene_ = &scene;
    if (camera == nullptr) {
        scene_camera_ = new SceneCamera(scene.GetSceneCamera());
        scene_camera_->SetScreenSize(size().width(), size().height());
        scene_camera_->Orbit(glm::vec3(-1.0f, 1.0f, 0.0f), glm::radians(45.0f));
        scene_camera_->Orbit(glm::vec3(1.0f, 1.0f, -1.0f), glm::radians(-10.0f));
        scene_camera_->SetFocus(glm::vec3(0,0,0));
    }
    else {
        scene_camera_ = new SceneCamera(*camera);
        scene_camera_->SetScreenSize(size().width(), size().height());
        if (!scene_camera_->IsPerspective()) {
            glm::vec4 view = scene_camera_->GetView()*glm::vec4(0,0,1,0);
            if (std::abs(glm::dot(view, glm::vec4(1,0,0,0))) > std::numeric_limits<float>::epsilon()) {
                camera_tool->setDefaultAction(ortho_x_action);
            } else if (std::abs(glm::dot(view, glm::vec4(0,1,0,0))) > std::numeric_limits<float>::epsilon()) {
                camera_tool->setDefaultAction(ortho_y_action);
            } else if (std::abs(glm::dot(view, glm::vec4(0,0,1,0))) > std::numeric_limits<float>::epsilon()) {
                camera_tool->setDefaultAction(ortho_z_action);
            }
        }
    }
    translator_.SetCamera(scene_camera_);

    scene.CameraCreated.Connect(this, &SceneWindow::OnCameraAdded);
    scene.SceneObjectDeleted.Connect(this, &SceneWindow::OnCameraRemoved);

    // If the SceneWindow is on RenderCam mode, set to not.
    if (render_cam_ != nullptr) {
        render_cam_ = nullptr;
        camera_tool->setDefaultAction(perspective_action);
    }

    // Clear RenderCams Menu
    render_cam_menu->clear();

    // Set a default texture for the renderer
    Texture& default_texture = scene_->GetAssetManager().GetDefaultTexture();
    renderer_->SetDefaultTexture(&default_texture);

    if (!gizmo_manager_) {
        gizmo_manager_ = new GizmoManager();
        connect(this, &SceneWindow::ObjectSelected, gizmo_manager_, &GizmoManager::OnObjectSelected);
        connect(this, &SceneWindow::ObjectTranslationBegin, gizmo_manager_, &GizmoManager::TranslationBegin);
        connect(this, &SceneWindow::ObjectTranslated, gizmo_manager_, &GizmoManager::ObjectTranslated);
        connect(this, &SceneWindow::ObjectRotationBegin, gizmo_manager_, &GizmoManager::RotationBegin);
        connect(this, &SceneWindow::ObjectRotated, gizmo_manager_, &GizmoManager::ObjectRotated);
        connect(this, &SceneWindow::ObjectScaleBegin, gizmo_manager_, &GizmoManager::ScaleBegin);
        connect(this, &SceneWindow::ObjectScaled, gizmo_manager_, &GizmoManager::ObjectScaled);
        scene.SceneObjectDeleted.Connect(gizmo_manager_, &GizmoManager::OnObjectDeleted);
    }
    gizmo_manager_->InitializeGizmos(scene_);

    Redraw();
}

void SceneWindow::initializeGL() {
    renderer_->Initialize();
    InitializeFrameBuffers();
}

void SceneWindow::InitializeFrameBuffers() {
    // TODO: Move to this renderer
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &framebuffer_tex_);
    glGenTextures(1, &render_framebuffer_tex_);
    glBindTexture(GL_TEXTURE_2D, framebuffer_tex_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, render_framebuffer_tex_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &framebuffer_depth_);
    glGenRenderbuffers(1, &render_framebuffer_depth_);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer_depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glBindRenderbuffer(GL_RENDERBUFFER, render_framebuffer_depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &fbo_);
    glGenFramebuffers(1, &render_fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_tex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer_depth_);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_framebuffer_tex_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_framebuffer_depth_);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    if (buffer_image_) {
        delete [] buffer_image_;
        delete [] saved_image_;
    }
    QRect rect = QApplication::desktop()->availableGeometry();
    bufsize = 4*rect.width()*rect.height();
    buffer_image_ = new float[bufsize];
    saved_image_ = new uchar[bufsize];
}

void SceneWindow::resizeGL(int width, int height) {
    if (scene_ == nullptr) return;

    // Update framebuffer sizes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, render_framebuffer_tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer_depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, render_framebuffer_depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (4*width*height > bufsize) {
        while (4*width*height > bufsize) {
            bufsize = 2*bufsize;
        }
        if (buffer_image_) {
            delete [] buffer_image_;
            delete [] saved_image_;
        }
        buffer_image_ = new float[bufsize];
        saved_image_ = new uchar[bufsize];
    }

    // Update projection matrix and other size related settings:
    scene_camera_->SetScreenSize(width, height);
    Redraw();
}

void SceneWindow::Redraw() {
//    #ifdef __APPLE__
//        update();
//    #endif
    needs_update_=true;
} //update(); }

void SceneWindow::SetFocus(glm::vec3 world_position) {
    focus_start_ = scene_camera_->GetAnchorPosition();
    focus_end_ = world_position;
    focus_time_ = 0.0;
    timer_->start(17);
}

SceneObject* SceneWindow::FindVertexController() {
    SceneObject* vertex_controller_e = scene_->GetSceneRoot().FindDescendant(&IsVertexController);
    if (vertex_controller_e == nullptr) {
        SceneObject& vertex_controller = scene_->CreateSceneObject("vertex");
        vertex_controller.SetFlag(SceneObject::INTERNAL_SELECTABLE);
        return &vertex_controller;
    } else {
        return vertex_controller_e;
    }
}

void SceneWindow::RemoveVertexController() {
    std::vector<SceneObject*> vertex_controllers = scene_->GetSceneRoot().FilterDescendants(&IsVertexController);
    for (auto vertex_controller : vertex_controllers) {
        scene_->DeleteSceneObject(vertex_controller->GetUID());
    }
}


void SceneWindow::paintGL() {
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Clear the buffer
    renderer_->Clear();

    //the scene will be invalid while this is happening
    if (SceneManager::Instance()->IsLoading()) {
        return;
    }

    if (scene_ == nullptr) return;

    // Prepare the renderer to draw from the scene
    renderer_->Setup(*scene_);
    renderer_->RenderAllEnvMaps(scene_->GetSceneRoot());
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0,0,width()*QApplication::desktop()->devicePixelRatio(),height()*QApplication::desktop()->devicePixelRatio());

    glm::mat4 view, proj;
    glm::vec2 dimensions;
    if (render_cam_ != nullptr) {
        view = glm::inverse(render_cam_->GetModelMatrix());
        Camera* camera = render_cam_->GetComponent<Camera>();
        unsigned int render_cam_width = camera->RenderWidth.Get();
        unsigned int render_cam_height = camera->RenderHeight.Get();
        camera->RenderWidth.Set(width());
        camera->RenderHeight.Set(height());
        proj = render_cam_->GetComponent<Camera>()->GetProjection();
        camera->RenderWidth.Set(render_cam_width);
        camera->RenderHeight.Set(render_cam_height);
        dimensions = glm::vec2(width(), height());
    } else {
        view = scene_camera_->GetView();
        proj = scene_camera_->GetProjection();
        dimensions = scene_camera_->GetScreenSize();
    }

    // Render the scene root
    renderer_->SetMaterialOverride(nullptr);
    renderer_->RenderNode(scene_->GetSceneRoot(), view, proj, dimensions);

    // Render the selected object
    if (render_selected_ && selected_object_ > 0) {
        // glClear(GL_DEPTH_BUFFER_BIT);
        SceneObject* scene_object = scene_->FindSceneObject(selected_object_);
        if (IsVertexController(scene_object)) {
            scene_object = scene_object->GetParent();
        }
        // This could happen if the object was deleted while selected
        if (scene_object != nullptr) {
            Material* material = scene_->GetAssetManager().GetMaterial("_internal Wireframe Material");
            assert(material != nullptr);
            renderer_->SetMaterialOverride(material);
            renderer_->SetRenderingSelection(true);
            renderer_->RenderNode(*scene_object, view, proj, dimensions);
            renderer_->SetRenderingSelection(false);
        }
    }

    // Render Normals
    if (render_normals_) {
        Material* material = scene_->GetAssetManager().GetMaterial("_internal Normals Material");
        assert(material != nullptr);
        renderer_->SetMaterialOverride(material);
        renderer_->RenderNode(scene_->GetSceneRoot(), view, proj, dimensions);
    }
    renderer_->SetMaterialOverride(nullptr);

    gizmo_manager_->SetTranslatorEnabled(false);
    gizmo_manager_->SetRotatorEnabled(false);
    gizmo_manager_->SetScalerEnabled(false);

    // Render Gizmos
    if (render_cam_ == nullptr && mode_ != MOUSEMODE_CAMERA && selected_object_ > 0) {
        assert(gizmo_manager_ != nullptr);
        SceneObject* scene_object = scene_->FindSceneObject(selected_object_);
        if (scene_object != nullptr) {
            gizmo_manager_->SetTransforms(scene_object, scene_camera_, mode_==MOUSEMODE_SCALE?Space::Local:manipulation_space_);
            gizmo_manager_->SetTranslatorEnabled(mode_ == MOUSEMODE_TRANSLATE);
            gizmo_manager_->SetRotatorEnabled(mode_ == MOUSEMODE_ROTATE);
            gizmo_manager_->SetScalerEnabled(mode_ == MOUSEMODE_SCALE);

            // Render
            gizmo_manager_->SetEnabled(true);
            glClear(GL_DEPTH_BUFFER_BIT);
            renderer_->SetNodePrefix("_gizmo");
            renderer_->RenderNode(scene_->GetSceneRoot(), scene_camera_->GetView(), scene_camera_->GetProjection(), scene_camera_->GetScreenSize());
            renderer_->SetNodePrefix("");
            gizmo_manager_->SetEnabled(false);

            // Highlight the current manipulator
            Material* material = scene_->GetAssetManager().GetMaterial("_mouse Highlight Material");
            assert(material != nullptr);
            renderer_->SetMaterialOverride(material);
            glClear(GL_DEPTH_BUFFER_BIT);
            SceneObject* clicked_gizmo = gizmo_manager_->GetClickedManipulator();
            if (clicked_gizmo != nullptr) renderer_->RenderNode(*clicked_gizmo, scene_camera_->GetView(), scene_camera_->GetProjection(), scene_camera_->GetScreenSize());
        }
    }

    // TODO: Move to this renderer
    // Render pick buffer

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0,0,width(),height());
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Material* material = scene_->GetAssetManager().GetMaterial("_mouse Color Picking Material");
    assert (material != nullptr);
    renderer_->SetMaterialOverride(material);
    renderer_->RenderNode(scene_->GetSceneRoot(), view, proj, dimensions);
    if (render_cam_ == nullptr && mode_ != MOUSEMODE_CAMERA && selected_object_ > 0) {
        gizmo_manager_->SetEnabled(true);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderer_->SetNodePrefix("_gizmo");
        renderer_->RenderNode(scene_->GetSceneRoot(), view, proj, dimensions);
        renderer_->SetNodePrefix("");
        gizmo_manager_->SetEnabled(false);
    }
    glReadPixels(0,0,width(),height(),GL_RGBA,GL_FLOAT,buffer_image_);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    renderer_->SetMaterialOverride(nullptr);
}

void SceneWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) { return; }
    if (cameraControls.find(event->key()) != cameraControls.end()) {
        keys_down_.insert(event->key());
    } else {
        QOpenGLWidget::keyPressEvent(event);
    }
}

void SceneWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) { return; }
    if (cameraControls.find(event->key()) != cameraControls.end()) {
        keys_down_.erase(event->key());
    } else {
        QOpenGLWidget::keyPressEvent(event);
    }
}

void SceneWindow::mouseDoubleClickEvent(QMouseEvent *) {
    if (scene_ == nullptr) return;
}

void SceneWindow::mouseMoveEvent(QMouseEvent *event) {
    if (scene_ == nullptr || render_cam_ != nullptr) return;

    Qt::MouseButtons buttons = event->buttons();
    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (manipulating_camera_) {
        glm::vec3 axis; glm::vec3 translation; glm::vec3 translation_sticky; double angle;
        trackball_.RotationUpdate(event->x(), height() - event->y(), width(), height(), axis, angle);
        trackball_.TranslationUpdate(-event->x(), event->y(), translation);
        // Keeps the mouse cursor affixed to the object
        if (clicked_object_ || !scene_camera_->IsPerspective()) {
            translator_.TranslationUpdate(event->x(), height() - event->y() - 1, translation_sticky);
            translation_sticky = -translation_sticky;
        }

        // Orbit: RMB or RMB + ALT for allow rolling
        // Zoom: LMB + ALT or Scroll Wheel
        // Move: MMB (or arrow keys, pgup+pgdn)
        if (buttons & Qt::RightButton) {
            if (modifiers & Qt::AltModifier) {
                scene_camera_->Orbit(axis, angle * rotation_speed_);
            } else {
                scene_camera_->EulerRotate((lastmousex-event->x())*euler_rotation_speed_, (lastmousey-event->y())*euler_rotation_speed_);
            }
        } else if (buttons & Qt::LeftButton) {
            if (modifiers & Qt::AltModifier) scene_camera_->ZoomRelative((translation.x + translation.y) * zoom_speed_ * 5.0f);
        } else if (buttons & Qt::MiddleButton) {
            // scene_camera_->Pan(axis, angle * rotation_speed_);
            if (clicked_object_ || !scene_camera_->IsPerspective()) scene_camera_->Move(translation_sticky, Space::World);
            else scene_camera_->Move(translation * translation_speed_, Space::Local);
        }
    } else if (mode_ == MOUSEMODE_TRANSLATE) {
        emit ObjectTranslated(event->x(), height() - event->y() - 1);
    } else if (mode_ == MOUSEMODE_ROTATE) {
        emit ObjectRotated(event->x(), height() - event->y() - 1);
    } else if (mode_ == MOUSEMODE_SCALE) {
        emit ObjectScaled(event->x(), height() - event->y() - 1);
    }
    mouse_moved_ = true;

    lastmousex = event->x();
    lastmousey = event->y();

    Redraw();
}

void SceneWindow::OnObjectSelected(uint64_t selected_object) {
    //Delete vertex controller when it is deselected
    if (selected_object > 0 && !gizmo_manager_->IsGizmo(selected_object)) {
        SceneObject* scene_object = scene_->FindSceneObject(selected_object);
        if (!IsVertexController(scene_object)) {
            RemoveVertexController();
        }
    }

    selected_object_ = selected_object;
    if (gizmo_manager_ != nullptr) gizmo_manager_->OnObjectSelected(selected_object);
}

void SceneWindow::SetVertexEditing(bool edit) {
    vertex_editing_ = edit;
    renderer_->SetVertexEditing(edit);
    if (!edit) {
        SceneObject* actual_scene_object = scene_->FindSceneObject(selected_object_);
        if (IsVertexController(actual_scene_object)) {
            uint64_t uid = actual_scene_object->GetParent()->GetUID();
            emit ObjectSelected(uid);
        }
    }
}

void SceneWindow::mousePressEvent(QMouseEvent *event) {
    lastmousex = event->x();
    lastmousey = event->y();

    if (scene_ == nullptr) return;
    Qt::KeyboardModifiers modifiers = event->modifiers();
    manipulating_camera_ = !(event->buttons() & Qt::LeftButton) || (modifiers & Qt::AltModifier);
    int idx = event->x() + width()*(height() - event->y());

    uint64_t uid = 0;

    uint64_t pick_buffer_uid = Util::ftoi(buffer_image_[4*idx+3]);

    if (selected_object_ > 0 && !gizmo_manager_->IsGizmo(selected_object_) && !gizmo_manager_->IsGizmo(pick_buffer_uid)) {
        SceneObject* actual_scene_object = scene_->FindSceneObject(selected_object_);
        SceneObject* scene_object = actual_scene_object;
        if (IsVertexController(actual_scene_object)) {
            scene_object = actual_scene_object->GetParent();
        }
        if (scene_object) {
            TriangleMesh* g = scene_object->GetComponent<TriangleMesh>();
            if (g && vertex_editing_) {
                glm::mat4 local_to_eye = scene_object->GetComponent<Transform>()->GetMatrix();
                SceneObject* obj = scene_object->GetParent();
                while(obj) {
                    local_to_eye = obj->GetComponent<Transform>()->GetMatrix() * local_to_eye;
                    obj = obj->GetParent();
                }

                glm::vec3 p, v;
                scene_camera_->GetRay(glm::vec2(event->x(), height() - event->y() - 1), p, v);

                local_to_eye = glm::translate(glm::mat4(), -p) * local_to_eye;

                int best_idx = -1;
                float best_dist = 0;

                const std::vector<float>& verticies = g->MeshFilter.Get()->GetPositions();

                for (int i=0; i<(verticies.size()/3); i++) {
                    glm::vec3 vtx = local_to_eye * glm::vec4(verticies[i*3],verticies[i*3+1],verticies[i*3+2], 1);
                    float dist = glm::dot(v,vtx);

                    vtx = vtx - (dist*v);


                    if (glm::length(vtx) < (dist*0.03)) {
                        if (best_idx == -1 || best_dist > dist) {
                            best_idx = i;
                            best_dist = dist;
                        }
                    }

                }

                if (best_idx != -1) {
                    SceneObject* vertex_controller_ = FindVertexController();
                    Transform* t = vertex_controller_->GetComponent<Transform>();
                    vertex_controller_->SetParent(*scene_object);
                    t->SetVertexTracking(g->MeshFilter.Get(), best_idx);
                    uid = vertex_controller_->GetUID();
                    SetMode(MOUSEMODE_TRANSLATE);
                }
            }
        }

        // If we're vertex editing, and a vertex isn't clicked, select the edited object
        if (vertex_editing_ && uid == 0) {
            if (IsVertexController(actual_scene_object)) {
                if (scene_object) {
                    uid = scene_object->GetUID();
                }
            }
        }
    }

    if (uid == 0) {
        uid = pick_buffer_uid;
    }

    glm::vec3 position(
            buffer_image_[4*idx],
            buffer_image_[4*idx+1],
            buffer_image_[4*idx+2]
    );

    if (uid == 0) {
        clicked_object_ = false;
        emit ObjectSelected(uid);
    } else {
        clicked_object_ = true;
        if (!manipulating_camera_) {
            emit ObjectSelected(uid);
            if (mode_ == MOUSEMODE_TRANSLATE) {
                emit ObjectTranslationBegin(scene_camera_, position, uid);
            }
            else if (mode_ == MOUSEMODE_ROTATE) {
                emit ObjectRotationBegin(scene_camera_, position, uid);
            }
            else if (mode_ == MOUSEMODE_SCALE) {
                emit ObjectScaleBegin(scene_camera_, position, uid);
            }
        }
    }
    if (render_cam_ == nullptr && manipulating_camera_) {
        trackball_.RotationBegin(event->x(), height() - event->y(), width(), height());
        trackball_.TranslationBegin(-event->x(), event->y());
        if (clicked_object_) {
            translator_.TranslationBegin(position, glm::vec4(0,0,0,0));
        } else if (!scene_camera_->IsPerspective()) {
            glm::vec3 p, v;
            scene_camera_->GetRay(glm::vec2(event->x(), height() - event->y() - 1), p, v);
            glm::vec4 towards = scene_camera_->GetTransform()*glm::vec4(0,0,-1,0);
            glm::vec4 translation_plane = glm::vec4(towards.xyz(), 0);
            glm::vec3 pos = Translator::RayPlaneIntersection(p, v, translation_plane);
            glm::vec4 camspace_pos = glm::inverse(scene_camera_->GetTransform())*glm::vec4(pos,1);
            translator_.TranslationBegin(camspace_pos.xyz(), translation_plane);
        }
    }
    Redraw();
}

void SceneWindow::mouseReleaseEvent(QMouseEvent *) {
    if (scene_ == nullptr) return;
    if (!clicked_object_ && !mouse_moved_ && !manipulating_camera_) emit ObjectSelected(0);
    mouse_moved_ = false;
    gizmo_manager_->ManipulationDone();
    Redraw();
}

void SceneWindow::wheelEvent(QWheelEvent *event) {
    if (render_cam_ != nullptr || scene_ == nullptr) return;
    if (event->buttons() & Qt::MiddleButton) return;

    scene_camera_->ZoomRelative(-event->angleDelta().y() * zoom_speed_);

    Redraw();
}

void SceneWindow::SetOrthoX() {
    render_cam_ = nullptr;
    scene_camera_->ToOrthographic();
    scene_camera_->SetRotation(glm::vec3(0,90,0));
    camera_tool->setDefaultAction(ortho_x_action);
    Redraw();
}
void SceneWindow::SetOrthoY() {
    render_cam_ = nullptr;
    scene_camera_->ToOrthographic();
    scene_camera_->SetRotation(glm::vec3(-90,0,0));
    camera_tool->setDefaultAction(ortho_y_action);
    Redraw();
}
void SceneWindow::SetOrthoZ() {
    render_cam_ = nullptr;
    scene_camera_->ToOrthographic();
    scene_camera_->SetRotation(glm::vec3(0,180,0));
    camera_tool->setDefaultAction(ortho_z_action);
    Redraw();
}

void SceneWindow::SetRenderCam(SceneObject* render_cam) {
    render_cam_ = render_cam;
    render_cam_menu->setDefaultAction(cam2action[render_cam]->action);
    Redraw();
}

void SceneWindow::OnCameraAdded(SceneObject& camera) {
    CameraMenuItem* item = new CameraMenuItem(camera, this);
    cam2action[&camera] = item;
    action2cam[item] = &camera;
    render_cam_menu->addAction(item->action);
}

void SceneWindow::OnCameraRemoved(SceneObject& camera) {
    if (camera.GetComponent<Camera>() != nullptr) {
        auto it = cam2action.find(&camera);
        if (it != cam2action.end()) {
            CameraMenuItem* item = it->second;
            cam2action.erase(&camera);
            action2cam.erase(item);
            render_cam_menu->removeAction(item->action);
            delete item;
        }
    }
}
