/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <glinclude.h>
#include <QOpenGLWidget>
#include <animator.h>
#include <qopenglshaderprogram.h>
#include <qopenglbuffer.h>
#include <qopengltextureblitter.h>
#include <QMouseEvent>
#include <opengl/glrenderer.h>
#include <trace/raytracer.h>

class Scene;
class SceneObject;

class RenderView : public QOpenGLWidget {
    Q_OBJECT

public:
    RenderView(QWidget* parent = nullptr);
    void SaveFrame(Scene& scene, SceneObject& rendercam, std::string output_filename, bool trace);
    void Cancel();

    void mousePressEvent(QMouseEvent *event);

    SceneObject* render_cam_;

public slots:
        void ContextChanged(bool);
private:
    std::unique_ptr<GLRenderer> renderer_;
    Scene* scene_;

    bool trace_;
    std::unique_ptr<QOpenGLTextureBlitter> blitter_;
    std::unique_ptr<RayTracer> tracer_;

    void initializeGL() override;
    void paintGL() override;
};

#endif // RENDERVIEW_H
