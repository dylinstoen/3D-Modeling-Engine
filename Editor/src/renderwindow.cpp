/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "renderwindow.h"
#include "mainwindow.h"
#include <scene/scene.h>
#include <scene/sceneobject.h>
#include <scene/components/camera.h>
#include <scene/renderer.h>
#include <opengl/glrenderer.h>
#include <iomanip>
#include <sstream>
#include <string>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QStyle>
#include <QDesktopServices>

// Pads a number with leading zeroes
std::string ZeroPadNumber(int num, unsigned int width = 5) {
    std::ostringstream ss;
    ss << std::setw(width) << std::setfill('0') << num;
    std::string result = ss.str();
    if (result.length() > width) {
        result.erase(0, result.length() - width);
    }
    return result;
}

RenderWindow::RenderWindow(QWidget *parent) :
    QDockWidget("Render View", parent),
    rendering_(false),
    first_view_(true)
{
    QWidget* new_render_widget = new QWidget(parent);
    new_render_widget->setMinimumSize(400,300);
    new_render_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* new_layout = new QVBoxLayout();
    new_layout->setMargin(0);
    new_layout->setSpacing(0);
    new_render_widget->setLayout(new_layout);

    QScrollArea* container = new QScrollArea(new_render_widget);
    container->setWidget(&render_view_);
    new_layout->addWidget(container);
    setWidget(new_render_widget);
    connect(this, &RenderWindow::topLevelChanged, &render_view_, &RenderView::ContextChanged);
    hide();
}

RenderWindow::~RenderWindow() {
}

void RenderWindow::closeEvent(QCloseEvent *event) {
    // Asynchronously closes the window and notifies rendering thread to stop
    rendering_ = false;
    QDockWidget::closeEvent(event);
    render_view_.Cancel();
}

int RenderWindow::exec(Scene& scene, const AnimationSettings& settings)
{
    switch (settings.Mode)
    {
    case AnimationSettings::AS_NORMAL:
        return exec_normal(scene, settings);
        break;

    case AnimationSettings::AS_DIFF:
        return exec_diff(scene, settings);
        break;

    default:
        break;
    }

    return 0;
}

int RenderWindow::exec_normal(Scene& scene, const AnimationSettings& settings) {
    if (first_view_) {
        setFloating(true);
        move(100,100);
        first_view_ = false;
    }
    std::string filename = settings.Filename;
    if (filename != "") {
        if (filename.substr(filename.length()-4,std::string::npos) == ".png") {
            filename = filename.substr(0, filename.length()-4);
        }
    }

    // Sanity checks
    SceneObject* render_cam = scene.GetOrCreateRenderCam();
    Q_ASSERT(render_cam != nullptr);

    if (settings.FPS == 0) {
        Debug::Log.WriteLine("Cannot save frames. FPS cannot be 0.", Priority::Error);
        return 1;
    }

    Camera* camera = render_cam->GetComponent<Camera>();
    if (camera->RenderWidth.Get() <= 0 || camera->RenderHeight.Get() <= 0) {
        Debug::Log.WriteLine("Cannot save frames. Width or Height cannot be 0.", Priority::Error);
        return 1;
    }

    // Show the window
    rendering_ = true;
    show();

    // Resize the window
    int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
//  int titleBarMargin = style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

    //Unset this in case it refers to a deleted object
    //TODO: use weak_ptr or something to fix dangling pointers to deleted objects
    render_view_.render_cam_ = nullptr;

    render_view_.setFixedSize(camera->RenderWidth.Get(), camera->RenderHeight.Get());
    resize(camera->RenderWidth.Get() + 4, camera->RenderHeight.Get() + titleBarHeight);

    // Save out all the frames

    if (settings.Length == 0) {
        setWindowTitle(QString::fromStdString("Frame"));
        std::string fn = filename;
        render_view_.SaveFrame(scene, *render_cam, fn, settings.Trace);
    } else {
        unsigned int total_frames = settings.FPS * settings.Length;
        double frame_time = 1.0 / settings.FPS;
        double current_time = 0.0;
        scene.Start();
        for (unsigned int current_frame = 0; rendering_ && current_frame < total_frames; current_frame++) {
            scene.Update(current_time, frame_time);
            std::string fn = filename;
            if (fn != "") {
                fn = fn + "_" + ZeroPadNumber(current_frame);
            }
            render_view_.SaveFrame(scene, *render_cam, fn, settings.Trace);
            setWindowTitle(QString::fromStdString("Saving frames (" + std::to_string(current_frame) + " of " + std::to_string(total_frames) + ")"));
            current_time += frame_time;
        }
        rendering_ = false;
        scene.Stop();
        scene.Reset();
    }

    // Close the window
    //hide();
    setWindowTitle(QString::fromStdString("Rendering complete!"));
    return 0;
}

int get_render_depth(Scene& scene)
{
    SceneObject* render_cam = scene.GetOrCreateRenderCam();
    Q_ASSERT(render_cam != nullptr);

    Camera* cam = render_cam->GetComponent<Camera>();
    return cam->TraceMaxDepth.Get();
}

MainWindow* RenderWindow::get_main_wnd()
{
    QObject* q = this;
    while (q->parent()!=nullptr) {
        q = q->parent();
    }
    return dynamic_cast<MainWindow*>(q);
}

std::string get_truth_image_path(const std::string &scene_file_path, int render_depth)
{
    // remove ".yaml"
    std::string scene_base_path = scene_file_path.substr(0, scene_file_path.length()-5);

    size_t base_name_start_pos = scene_base_path.find_last_of("\/");
    if (base_name_start_pos == std::string::npos)
        base_name_start_pos = 0;

    std::string base_name = scene_base_path.substr(base_name_start_pos, std::string::npos);

    return std::string("assets/trace/solution") + base_name + std::string("_") + std::to_string(render_depth) + std::string(".png");
}

std::string get_diff_image_path(const std::string &scene_file_path, int render_depth)
{
    // remove ".yaml"
    std::string scene_base_path = scene_file_path.substr(0, scene_file_path.length() - 5);
    return scene_base_path + std::string("_") + std::to_string(render_depth) + std::string("_diff") + std::string(".png");
}

SceneObject* RenderWindow::set_render_view(Scene &scene, int &render_width, int &render_height)
{
    SceneObject* render_cam = scene.GetOrCreateRenderCam();
    Q_ASSERT(render_cam != nullptr);
    Camera* camera = render_cam->GetComponent<Camera>();

    int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
    render_view_.render_cam_ = nullptr;
    render_view_.setFixedSize(camera->RenderWidth.Get(), camera->RenderHeight.Get());
    resize(camera->RenderWidth.Get() + 4, camera->RenderHeight.Get() + titleBarHeight);

    setWindowTitle(QString::fromStdString("Render Result"));

    rendering_ = true;
    show();

    render_width = camera->RenderWidth.Get();
    render_height = camera->RenderHeight.Get();

    return render_cam;
}

int RenderWindow::exec_diff(Scene& scene, const AnimationSettings& settings)
{
    if (first_view_) {
        setFloating(true);
        move(100,100);
        first_view_ = false;
    }

    int render_width = -1, render_height = -1;
    SceneObject* render_cam = set_render_view(scene, render_width, render_height);
    assert(render_width != -1);
    assert(render_height != -1);

    render_view_.SaveFrame(scene, *render_cam, "", settings.Trace);

    MainWindow* p_mainwnd = get_main_wnd();
    assert(p_mainwnd);

    std::string scene_file_path = p_mainwnd->current_file_path_.c_str();
    int render_depth = get_render_depth(scene);

    Debug::Log.WriteLine(scene_file_path + std::string(" [render depth = ") + std::to_string(render_depth) + std::string("]"));

    std::string truth_img_path = get_truth_image_path(scene_file_path, render_depth);
    QImage truth_image(QString::fromStdString(truth_img_path));
    if (truth_image.isNull())
    {
        Debug::Log.WriteLine("...... No Ground Truth Image");
        return 0;
    }

    QImage render_result = render_view_.grabFramebuffer();
    if ((render_result.width() != render_width) || (render_result.height() != render_height))
        render_result = render_result.scaled(render_width, render_height);

    if ((render_result.width() != truth_image.width()) || (render_result.height() != truth_image.height()))
    {
        Debug::Log.WriteLine("...... The size of rendering result and truth image is not consistent.");
        return 0;
    }

    int width = truth_image.width(), height = truth_image.height();

    QImage diff_image(width, height, truth_image.format());
    unsigned int error_sum = 0;
    for (int y = 0; y < height; y++)
    {
        const uchar *p_render_data = render_result.scanLine(y);
        const uchar *p_truth_data = truth_image.scanLine(y);
        uchar *p_diff_data = diff_image.scanLine(y);
        for (int x = 0; x < width; x++)
        {
            // pixel layout: BGRA

            unsigned int error = 0;
            for (int c = 0; c < 3; c++)
                error += (unsigned int)std::abs(p_render_data[c] - p_truth_data[c]);

            p_diff_data[0] = 0;
            p_diff_data[1] = 0;
            p_diff_data[2] = (error > 0)? 255: 0;
            p_diff_data[3] = p_truth_data[3];

            p_render_data += 4;
            p_truth_data += 4;
            p_diff_data += 4;

            error_sum += error;
        }
    }

    std::string result = std::string("...... Error: ") + std::to_string((double)error_sum / (width * height));
    Debug::Log.WriteLine(result);

    std::string diff_img_path = get_diff_image_path(scene_file_path, render_depth);
    diff_image.save(QString::fromStdString(diff_img_path));

    if (settings.isOpenDiffImage)
        QDesktopServices::openUrl(QUrl(("file:///" + diff_img_path).c_str(), QUrl::TolerantMode));

    return 0;
}
