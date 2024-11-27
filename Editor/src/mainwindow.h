/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <animator.h>
#include <opengl/glslshaderfactory.h>
#include <properties.h>
#include <scenewindow.h>
#include <assets/assetbrowser.h>
#include <hierarchy/hierarchyview.h>
#include <inspector/inspector.h>
#include <scene/scene.h>
#include <animation/animationwidget.h>
#include <qtwidgets.h>
#include <QMainWindow>
#include <QProcess>
#include <QSplitter>
#include <scene/scenemanager.h>
#include <scene/translator.h>
#include <actionmanager.h>
#include <filterdialog.h>
#include <curveeditorwindow.h>
#include <renderwindow.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    friend class RenderWindow;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void RedrawSceneViews() {
        for (auto& view : scene_views_) view->Redraw();
    }

protected:
    Scene* scene_;
    AssetBrowser assets_;
    HierarchyView hierarchy_;
    AnimationWidget animator_;
    Inspector inspector_;
    GLSLShaderFactory shader_factory_;
    SceneManager scene_manager_;
    FilterDialog filter_dialog_;
    CurveEditorWindow curve_editor_dialog_;
    RenderWindow render_window_;
    SceneWindow::MouseMode mode;
    bool vertex_editing;
    std::string current_file_path_;
    QString trace_path_;
    QProcess trace_process_;
    float previous_animation_time_;
    // TODO: Move this to renderer?
    SceneObject* grid_;
    SceneObject* selected_object_;

    // UI Stuff
    Ui::MainWindow *ui;
    ActionManager actions_;
    std::vector<SceneWindow*> scene_views_;
    QSplitter* vsplit_;
    QSplitter* hsplit1_;
    QSplitter* hsplit2_;
    QMenu* hierarchy_context_menu_;

    void ShowSplit();
    void HideSplit();
    void CheckSplitState();

    // Notification from the logger that user wants to write something
    void LogCallback(std::string message, Priority p);

    // Previous console messages may no longer apply (e.g. on refresh, scene load)
    void GrayoutConsole();

    // Prompts the user with a dialog asking to input a name.
    // Returns the result name, and if canceled returns an empty string.
    std::string GetNameInput(const std::string& title);

    // Creates an OpenGL Context, makes it shared, initializes the GLRenderer
    void InitializeContext();
    SceneWindow* AddSceneView(QWidget* parent, SceneCamera* camera = nullptr);
    void InitializeScenewindowSignals(SceneWindow* view);

    // Scene Management
    void NewScene();
    void OpenScene();
    void SaveScene();
    void SaveSceneAs();

    void RaytraceFrameNoSave();
    void RaytraceFrame();
    void RasterizeMovieFrames();
    void RaytraceMovieFrames();

    void DiffAllRaytraceScenes();
    void RaytraceFrameAndDiff();

    void SetScene(Scene& scene);

    // Single-Shot initialization
    void CreateSplitScreen();
    void CreateFileActions();
    void CreateRenderActions();
    void CreateViewActions();
    void CreateAssetsActions();
    void CreateSceneObjectsActions();
    void CreateManipulatorActions();
    void CreateAnimationActions();
    void CreateToolbar();
    void CreateConsoleToolbar();
    void CreateMenus();
};

#endif // MAINWINDOW_H
