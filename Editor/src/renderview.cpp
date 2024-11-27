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
#include "mainwindow.h"
#include "renderview.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCoreApplication>
#include <scene/scene.h>
#include <scene/sceneobject.h>
#include <scene/components/camera.h>
#include <opengl/glrenderer.h>
#include <trace/raytracer.h>

RenderView::RenderView(QWidget* parent) :
    QOpenGLWidget(parent),
    renderer_(std::make_unique<GLRenderer>()),
    scene_(nullptr),
    render_cam_(nullptr),
    trace_(false),
    tracer_(nullptr)
{
    renderer_->DisplayLights(false);
    renderer_->DisplayCamera(false);
    renderer_->DisplayColliders(false);
}

void RenderView::SaveFrame(Scene& scene, SceneObject& rendercam, std::string output_filename, bool trace) {
    scene_ = &scene;
    render_cam_ = &rendercam;
    trace_ = trace;

    if (trace) {
        // automatically starts drawing on other threads
        tracer_.reset(new RayTracer(scene, rendercam));

        //if window is closed, tracer_ is deleted
        while(tracer_!=nullptr && tracer_->GetProgress() < 100) {
            update();
            QCoreApplication::processEvents();
            QThread::msleep(5);
        }

        if (tracer_ == nullptr) {
            return;
        }
    }

    update();
    QCoreApplication::processEvents();
    if (output_filename != "") {
            QString rfile = QString::fromStdString(output_filename + ".png");
            Camera* cam = rendercam.GetComponent<Camera>();
            int rwidth = cam->RenderWidth.Get();
            int rheight = cam->RenderHeight.Get();
            QImage cnv = grabFramebuffer();
            if (cnv.width() == rwidth && cnv.height() == rheight) {
                cnv.save(rfile);
            } else {
                cnv.scaled(rwidth, rheight).save(rfile);
            }
        }
}

void RenderView::Cancel() {
    tracer_.reset(nullptr);
}

void RenderView::mousePressEvent(QMouseEvent *event)
{
    if (tracer_) {
        tracer_->ComputePixel(event->x(), height()-event->y(), render_cam_->GetComponent<Camera>());
    }

    QObject* q = this;
    while (q->parent()!=nullptr) {
        q = q->parent();
    }
    MainWindow* qw = dynamic_cast<MainWindow*>(q);
    assert(qw);
    qw->RedrawSceneViews();
}

void RenderView::ContextChanged(bool) {
    renderer_->ContextChanged();
    blitter_.reset();
    update();
}

void RenderView::initializeGL() {
    renderer_->Initialize();
}

void RenderView::paintGL() {
    if (scene_ == nullptr || render_cam_ == nullptr) return;
    if (!(blitter_.get())) {
        blitter_ = std::make_unique<QOpenGLTextureBlitter>();
        blitter_->create();
    }
    Camera* camera = render_cam_->GetComponent<Camera>();

    //draw rendering progress from tracebuffer
    if (trace_) {
        if (tracer_ == nullptr) return;

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
        glViewport(0,0, tracer_->settings.width * QApplication::desktop()->devicePixelRatio(), tracer_->settings.height * QApplication::desktop()->devicePixelRatio());

        // Clear before drawing
        glLoadIdentity();
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Enable Texture
        glColor3f(1,1,1);
        glEnable(GL_TEXTURE_2D);

        // Create texture
        GLuint tex;
        glGenTextures(1, &tex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //this took way too long to figure out

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tracer_->settings.width, tracer_->settings.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tracer_->buffer);
        // Set texture properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        blitter_->bind();
        blitter_->blit(tex, QOpenGLTextureBlitter::targetTransform(QRectF{0,0,(float)(tracer_->settings.width),(float)(tracer_->settings.height)},QRect{0,0,((int)tracer_->settings.width),((int)tracer_->settings.height)}), QOpenGLTextureBlitter::OriginBottomLeft);

        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &tex);

    } else {
        renderer_->Clear();
        renderer_->Setup(*scene_);
        renderer_->RenderAllEnvMaps(scene_->GetSceneRoot());
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
        glViewport(0,0, camera->RenderWidth.Get() * QApplication::desktop()->devicePixelRatio(), camera->RenderHeight.Get() * QApplication::desktop()->devicePixelRatio());

        glm::mat4 view = glm::inverse(render_cam_->GetModelMatrix());
        glm::mat4 proj = camera->GetProjection();
        glm::vec2 dimensions = glm::vec2(camera->RenderWidth.Get(), camera->RenderHeight.Get());

        renderer_->RenderNode(scene_->GetSceneRoot(), view, proj, dimensions);
    }
}
