/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QDockWidget>
#include <renderview.h>

class MainWindow;

struct AnimationSettings {
    enum AS_Mode
    {
        AS_NORMAL = 0,
        AS_DIFF
    };


    unsigned int FPS;
    unsigned int Length; //0 = one frame only
    std::string Filename; //empty string = don't save
    bool Trace;

    AS_Mode Mode;
    bool isOpenDiffImage;

    AnimationSettings(unsigned int fps_, unsigned int length_, std::string filename_, bool trace_, AS_Mode mode_ = AS_NORMAL, bool is_open_diff_=false)
        : FPS(fps_), Length(length_), Filename(filename_), Trace(trace_)
        , Mode(mode_), isOpenDiffImage(is_open_diff_){ }
};

class RenderWindow : public QDockWidget {
    Q_OBJECT

public:
    explicit RenderWindow(QWidget *parent = 0);
    ~RenderWindow();

    virtual void closeEvent(QCloseEvent* event) override;
    virtual int exec(Scene& scene, const AnimationSettings& settings);

    int exec_normal(Scene& scene, const AnimationSettings& settings);
    int exec_diff(Scene& scene, const AnimationSettings& settings);

private:
    RenderView render_view_;
    bool rendering_;
    bool first_view_;

    MainWindow* get_main_wnd();
    SceneObject* set_render_view(Scene& scene, int &render_width, int &render_height);
};

#endif // RENDERWINDOW_H
