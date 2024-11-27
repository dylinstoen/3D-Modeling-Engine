/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <animator.h>
#include <QOpenGLContext>
#include <mainwindow.h>
#include <scenewindow.h>
#include <ui_mainwindow.h>
#include <testwindow.h>
#include <qtwidgets.h>
#include <QString>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QOffscreenSurface>
#include <QProcess>
#include <fileio.h>
#include <opengl/glrenderer.h>
#include <meshprocessing.h>
#include <renderview.h>
#include <QTextBlock>
#include <QTextStream>
#include <QFile>

const bool ANIMATOR_ENABLED = true;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    scene_(nullptr),
    scene_manager_(shader_factory_, animator_.GetCurveFactory()),
    filter_dialog_(this),
    curve_editor_dialog_(nullptr),
    render_window_(this),
    vertex_editing(false),
#if defined(_WIN32) || defined (_WIN64)
    trace_path_(tr("Trace.exe")),
#else
    trace_path_(tr("Trace")),
#endif
    previous_animation_time_(0),
    grid_(nullptr),
    selected_object_(nullptr),
    ui(new Ui::MainWindow),
    actions_(this),
    hierarchy_context_menu_(new QMenu(tr("Hierarchy Context Menu"), this))
{
    // Creates the widgets from the .ui designer file
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);

    // Attach Listener to the Debug output
    Debug::Log.AttachListener("MainWindow", std::bind(&MainWindow::LogCallback, this, std::placeholders::_1, std::placeholders::_2));
    Debug::Log.WriteLine("Ready", Priority::Status);

    // Initialize OpenGLContext so we can create context specific resources
    InitializeContext();

    // Sets current dir (for Mac bug fix)
#ifdef __APPLE__
    QDir::setCurrent(QCoreApplication::applicationDirPath());
#endif
    Debug::Log.WriteLine("Current Working Directory: " + QDir::currentPath().toStdString());


    // Layout stuff
    ui->assetsLayout->setMargin(0);
    ui->assetsLayout->addWidget(&assets_);
    ui->hierarchyLayout->setMargin(0);
    ui->hierarchyLayout->addWidget(&hierarchy_);
    ui->inspectorLayout->setMargin(0);
    ui->scrollArea->setWidget(&inspector_);
    inspector_.setMinimumWidth(380);
    ui->animatorLayout->setMargin(0);
    ui->animatorLayout->addWidget(&animator_);

    // Event stuff
    connect(&hierarchy_, &HierarchyView::AssetPickerRequested, &assets_, &AssetBrowser::OnAssetPicker);
    connect(&hierarchy_, &HierarchyView::InspectableSelected, &inspector_, &Inspector::OnSelectInspectable);
    connect(&hierarchy_, &HierarchyView::InspectableAdded, &inspector_, &Inspector::OnAddInspectable);
    connect(&assets_, &AssetBrowser::InspectableSelected, &inspector_, &Inspector::OnSelectInspectable);
    connect(&assets_, &AssetBrowser::InspectableAdded, &inspector_, &Inspector::OnAddInspectable);

    // Dock Widget stuff
    setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
    setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);

    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    tabifyDockWidget(ui->assetsDock, ui->hierarchyDock);
    tabifyDockWidget(ui->animatorDock, ui->consoleDock);
    connect(ui->assetsDock, &QDockWidget::visibilityChanged, &assets_, [&, this](bool visible) {
        if (visible) assets_.OnChanged();
    });
    connect(ui->hierarchyDock, &QDockWidget::visibilityChanged, &hierarchy_, [&, this](bool visible) {
        if (visible) hierarchy_.OnChanged();
    });

    connect(ui->sceneDock, &QDockWidget::topLevelChanged, this, [&, this](bool toplevel) {
        for (int i = 0; i < 4; i++)
            scene_views_[i]->ContextChanged(toplevel);
    });

    if (!ANIMATOR_ENABLED) ui->animatorDock->setHidden(true);

    // Single-Shot Initialization
    CreateSplitScreen();
    CreateFileActions();
    CreateRenderActions();
    CreateViewActions();
    CreateAssetsActions();
    CreateSceneObjectsActions();
    CreateManipulatorActions();
    CreateToolbar();
    CreateConsoleToolbar();
    CreateMenus();



    QStringList args = QApplication::arguments();
    //qDebug() << args.length();


//    todo put trace cmdline here



    // Initialize default scene
    NewScene();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::ShowSplit() {
    QList<int> sizes;
    sizes.push_back(vsplit_->height()/2);
    sizes.push_back(vsplit_->height()/2);
    vsplit_->setSizes(sizes);
    sizes[0] = hsplit1_->width()/2;
    sizes[1] = hsplit1_->width()/2;
    hsplit1_->setSizes(sizes);
    hsplit2_->setSizes(sizes);
}

void MainWindow::HideSplit(){
    QList<int> vsizes, hsizes;
    vsizes.push_back(ui->sceneWidget->height());
    vsizes.push_back(0);
    vsplit_->setSizes(vsizes);
    hsizes.push_back(0);
    hsizes.push_back(ui->sceneWidget->width());
    hsplit1_->setSizes(hsizes);
    hsplit2_->setSizes(hsizes);
}

void MainWindow::CheckSplitState(){
    if (vsplit_->sizes()[1] == 0 && hsplit1_->sizes()[0] == 0)
        actions_["Split-Screen"]->setChecked(false);
    else if (vsplit_->sizes()[0] == vsplit_->sizes()[1] && hsplit1_->sizes()[0] == hsplit1_->sizes()[1])
        actions_["Split-Screen"]->setChecked(true);
}

void MainWindow::InitializeContext() {
    // Create a shared Context to make current so we can start allocating OpenGL resources
    QOpenGLContext* context = new QOpenGLContext();
    context->setShareContext(QOpenGLContext::globalShareContext());
    if (!context->create()) throw RenderingException("Unable to create MainWindow context");

    // Create an offscreen surface so there's something to make current to
    QOffscreenSurface* surface = new QOffscreenSurface();
    surface->create();
    if (!surface->isValid()) throw RenderingException("Unable to create MainWindow offscreen surface");

    // MakeCurrent and initialize renderer
    if (!context->makeCurrent(surface)) throw RenderingException("Unable to make MainWindow context current");
    GLRenderer::GlobalInitialize();
}

void MainWindow::LogCallback(std::string message, Priority p) {
    switch (p) {
        case Priority::Normal:
            ui->console->append(QString::fromStdString(message));
            break;
        case Priority::Warning: {
            std::string line = "WARN: " + message;
            ui->console->append(QString::fromStdString(line));
            break; }
        case Priority::Error: {
            std::string line = "ERROR: " + message;
            ui->console->append(QString::fromStdString(line));
            break; }
        case Priority::Status:
            statusBar()->showMessage(QString::fromStdString(message));
            break;
    }
}

void MainWindow::GrayoutConsole() {
    ui->console->selectAll();
    ui->console->setTextColor(Qt::gray);
    QTextCursor cursor(ui->console->document());
    cursor.clearSelection();
    cursor.setPosition(cursor.block().length());
    ui->console->setTextCursor(cursor);
    ui->console->setTextColor(Qt::black);
}

std::string MainWindow::GetNameInput(const std::string &title) {
    // Prompt the user to enter a name
    bool okay;
    QString name = QInputDialog::getText(this, tr(title.c_str()), tr("Enter a Name:"), QLineEdit::Normal, tr(""), &okay, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (!okay || name.isEmpty()) return "";
    return name.toStdString();
}

void MainWindow::InitializeScenewindowSignals(SceneWindow* view) {
    connect(&hierarchy_, &HierarchyView::ObjectSelected, view, &SceneWindow::OnObjectSelected);
    connect(&hierarchy_, &HierarchyView::RedrawRequested, view, &SceneWindow::Redraw);
    connect(&assets_, &AssetBrowser::RedrawRequested, view, &SceneWindow::Redraw);
    connect(view, SIGNAL(ObjectSelected(uint64_t)), &hierarchy_, SLOT(SelectObject(uint64_t)));
    connect(view, &SceneWindow::TearoffRequested, this, [this](SceneCamera* camera) {
        QDockWidget* new_dock = new QDockWidget("Scene View", this);
        SceneWindow* new_scene_widget = AddSceneView(new_dock, camera);
        new_scene_widget->setMinimumSize(360,240);
        new_dock->setWidget(new_scene_widget->parentWidget());
        new_dock->setFloating(true);
        new_dock->show();
        connect(new_dock, &QDockWidget::topLevelChanged, new_scene_widget, &SceneWindow::ContextChanged);
    });
}

void MainWindow::NewScene() {
    // Make a new default Scene and make sure it has correct default animation settings
    Scene& scene = *(scene_manager_.NewScene("Untitled Scene"));
    scene.SetFPS(animator_.GetFPS());
    scene.SetAnimationLength(animator_.GetAnimationLength());
    // Set the scene and put some basic stuff in it
    GrayoutConsole();
    SetScene(scene);
    scene_->GetOrCreateRenderCam();
    scene_->CreateSphere("Sphere");
    auto key_light = scene_->CreateDirectionalLight("Key Light");
    key_light.GetTransform().Rotation.Set(glm::vec3(45,45,0));
    glm::vec4 light_pos = key_light.GetTransform().GetMatrix()*glm::vec4(0,2,0,0);
    key_light.GetTransform().Translation.Set(light_pos.xyz());
    auto key_light_comp = key_light.GetComponent<Light>()->as<DirectionalLight>();
    key_light_comp->IntensityMultiplier.Set(1.5);
    auto fill_light = scene_->CreateDirectionalLight("Fill Light");
    fill_light.GetTransform().Rotation.Set(glm::vec3(-135, 45, 0));
    light_pos = fill_light.GetTransform().GetMatrix()*glm::vec4(0,2,0,0);
    fill_light.GetTransform().Translation.Set(light_pos.xyz());
    auto fill_light_comp = fill_light.GetComponent<Light>()->as<DirectionalLight>();
    fill_light_comp->Ambient.Set(glm::vec3(0.0f, 0.0f, 0.0f));
    fill_light_comp->Color.Set(glm::vec3(0.3f, 0.0f, 0.0f));
}

SceneWindow* MainWindow::AddSceneView(QWidget* parent, SceneCamera* camera) {
    QWidget* new_scene_widget = new QWidget(parent);
    new_scene_widget->setMinimumSize(360,240);
    new_scene_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* new_layout = new QVBoxLayout();
    new_layout->setMargin(0);
    new_layout->setSpacing(0);
    new_scene_widget->setLayout(new_layout);
    SceneWindow* new_scene_view = new SceneWindow(*new_layout, new_scene_widget);
    new_layout->addWidget(new_scene_view);
    InitializeScenewindowSignals(new_scene_view);
    if (scene_) new_scene_view->SetScene(*scene_, camera);
    SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
    if (!scene_views_.empty()) new_scene_view->SetMode(scene_views_[0]->GetMode());
    if (selected_object != nullptr) {
        new_scene_view->OnObjectSelected(selected_object->GetUID());
    }
    scene_views_.push_back(new_scene_view);
    return new_scene_view;
}

void MainWindow::OpenScene() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Scene"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Scene], 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    auto file_info = QFileInfo(filename);
    FilePicker::LastPath = file_info.path();
    Scene* new_scene = scene_manager_.LoadScene(filename.toStdString());
    if (new_scene != nullptr) {
        current_file_path_ = filename.toStdString();
        GrayoutConsole();
        SetScene(*new_scene);
    }
}
void MainWindow::SaveScene() {
    if (current_file_path_.empty()) SaveSceneAs();
    else scene_manager_.SaveScene(scene_->GetName() ,current_file_path_);
}

void MainWindow::SaveSceneAs() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Scene"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Scene], 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    if (!filename.endsWith(".yaml")) filename += ".yaml";
    QFileInfo file_info(filename);
    FilePicker::LastPath = file_info.path();
    current_file_path_ = filename.toStdString();
    scene_manager_.SaveScene(file_info.baseName().toStdString(), filename.toStdString());
}

void MainWindow::RaytraceFrameNoSave() {
    if (scene_ == nullptr) return;
    AnimationSettings settings(animator_.GetFPS(), 0, "", true);
    render_window_.exec(*scene_, settings);
}

void MainWindow::RaytraceFrame() {
    if (scene_ == nullptr) return;

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Frame As"), FilePicker::LastPath, "Image Files (*.png)", 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    AnimationSettings settings(animator_.GetFPS(), 0, filename.toStdString(), true);
    render_window_.exec(*scene_, settings);
}

void MainWindow::RasterizeMovieFrames() {
    if (scene_ == nullptr) return;

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As"), FilePicker::LastPath, "Image Files (*.png)", 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    AnimationSettings settings(animator_.GetFPS(), animator_.GetAnimationLength(), filename.toStdString(), false);
    render_window_.exec(*scene_, settings);
}

void MainWindow::RaytraceMovieFrames() {
    if (scene_ == nullptr) return;

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Frames As"), FilePicker::LastPath, "Image Files (*.png)", 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    AnimationSettings settings(animator_.GetFPS(), animator_.GetAnimationLength(), filename.toStdString(), true);
    render_window_.exec(*scene_, settings);
}

void MainWindow::RaytraceFrameAndDiff() {
    Debug::Log.WriteLine("\n\n============ Start Diff Evaluations ============");

    if (scene_ == nullptr) return;
    AnimationSettings settings(animator_.GetFPS(), 0, "", true, AnimationSettings::AS_DIFF, true);
    render_window_.exec(*scene_, settings);
}

bool is_yaml_file(const std::string file_path)
{
    if (file_path.length() < 5)
        return false;

    return file_path.substr(file_path.length() - 5, std::string::npos) == ".yaml";
}

void read_test_scene_paths(const char *config_path, std::vector<std::string> &scene_files)
{
    QFile file(config_path);
    if(!file.open(QIODevice::ReadOnly)) {
        qInfo() << "Configure file could not open.";
        return;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        scene_files.push_back(line.toStdString());
    }
    file.close();
}

void MainWindow::DiffAllRaytraceScenes()
{
    Debug::Log.WriteLine("\n\n============ Start Batch Diff Evaluations ============");

    std::string old_scene_file_path = current_file_path_;

    std::vector<std::string> scene_files;
    read_test_scene_paths("assets/trace/diff_scenes.txt", scene_files);
    if (scene_files.empty())
        return;

    for (size_t i = 0; i < scene_files.size(); i++)
    {
        std::string scene_file_path = scene_files[i];

        Scene* new_scene = scene_manager_.LoadScene(scene_file_path);
        if (new_scene != nullptr)
        {
            current_file_path_ = scene_file_path;
            GrayoutConsole();
            SetScene(*new_scene);
        }

        AnimationSettings settings(animator_.GetFPS(), 0, "", true, AnimationSettings::AS_DIFF);
        render_window_.exec(*scene_, settings);

        delete new_scene;
    }
    render_window_.close();

    if (is_yaml_file(old_scene_file_path))
    {
        Scene* new_scene = scene_manager_.LoadScene(old_scene_file_path);
        if (new_scene != nullptr)
        {
            GrayoutConsole();
            SetScene(*new_scene);
        }
    }
    else
    {
        NewScene();
    }

    current_file_path_ = old_scene_file_path;
}

void MainWindow::SetScene(Scene& scene) {
    scene_ = &scene;

    // Split-Screen Views
    for (auto& scenewindow : scene_views_) scenewindow->SetScene(scene);
    scene_views_[2]->SetOrthoX();
    scene_views_[3]->SetOrthoY();
    scene_views_[0]->SetOrthoZ();
    hierarchy_.SetScene(scene);
    assets_.SetScene(scene);

    // Populate RenderCams Menu
    for (auto& cam : scene.GetRenderCams()) {
        scene.CameraCreated.Emit(*cam);
    }

    // Zoom the camera out from the center
    scene_->GetSceneCamera().Zoom(3.0f);

    // Set the Animation Properties on Animator
    animator_.SetAnimationLength(scene_->GetAnimationLength());
    animator_.SetFPS(scene_->GetFPS());

    // Clear any signals dispatching to old scene
    hierarchy_.disconnect(this);
    animator_.disconnect(this);

    // Connect signals to new scene
    connect(&hierarchy_, &HierarchyView::ObjectParentChanged, this, [this](uint64_t object_id, uint64_t parent_id) {
        scene_->ReparentSceneObject(object_id, parent_id);
    });
    connect(&hierarchy_, &HierarchyView::ObjectDuplicated, this, [this](uint64_t object_id) {
        scene_->DuplicateSceneObject(object_id);
    });
    connect(&hierarchy_, &HierarchyView::ObjectDeleted, this, [this](uint64_t object_id) {
        scene_->DeleteSceneObject(object_id);
        if (selected_object_ != nullptr && object_id == selected_object_->GetUID()) animator_.ShowSceneObject(nullptr);
    });
    connect(&hierarchy_, &HierarchyView::ObjectFocused, this, [this](uint64_t object_id) {
        // Get world position of the object to focus on
        SceneObject* obj = scene_->FindSceneObject(object_id);
        glm::vec4 world_pos = obj->GetParentModelMatrix() * glm::vec4(obj->GetTransform().Translation.Get(), 1.0);
        for (auto view : scene_views_) {
            view->SetFocus(world_pos.xyz);
        }
    });
    connect(&hierarchy_, &HierarchyView::ObjectPropertiesChanged, this, [this](uint64_t object_id){
        SceneObject* obj = scene_->FindSceneObject(object_id);
        animator_.ShowSceneObject(obj);
    });
    connect(&hierarchy_, &HierarchyView::ObjectSelected, this, [this](uint64_t object_id){
        SceneObject* obj = scene_->FindSceneObject(object_id);
        if (selected_object_ == obj) {
            return;
        }
        selected_object_ = obj;
        animator_.ShowSceneObject(obj);
        // Disable filtering if there is no selected object with a mesh on it
        if (obj == nullptr) {
            actions_["Filter Selected"]->setDisabled(true);
            actions_["Flip Normals on Selected"]->setDisabled(true);
            actions_["Export Mesh"]->setDisabled(true);
        } else {
            Geometry* geo = obj->GetComponent<Geometry>();
            if (geo == nullptr || geo->GetRenderMesh() == nullptr) {
                actions_["Filter Selected"]->setDisabled(true);
                actions_["Flip Normals on Selected"]->setDisabled(true);
                actions_["Export Mesh"]->setDisabled(true);
            } else {
                TriangleMesh* trimesh = obj->GetComponent<TriangleMesh>();
                actions_["Filter Selected"]->setDisabled(trimesh == nullptr);
                actions_["Flip Normals on Selected"]->setDisabled(trimesh == nullptr);
                actions_["Export Mesh"]->setDisabled(false);
            }
        }
    });
    connect(&hierarchy_, &HierarchyView::ObjectCreated, this, [this](uint64_t object_id){
        // Needed so new properties are initialized with current frame time
        scene_->Update(previous_animation_time_, 0.0f);
    });

    connect(&hierarchy_, &HierarchyView::HierarchyContextOpen, this, [this](const QPoint& pos) {
        hierarchy_context_menu_->exec(pos);
    });

    connect(&animator_, &AnimationWidget::AnimationUpdate, this, [this](float t){
        float delta_t = t - previous_animation_time_;
        scene_->Update(t, delta_t);
        previous_animation_time_ = t;
        RedrawSceneViews();
    });
    connect(&animator_, &AnimationWidget::AnimationStart, this, [this](){
        scene_->Start();
    });
    connect(&animator_, &AnimationWidget::AnimationStop, this, [this](){
        scene_->Stop();
    });
    connect(&animator_, &AnimationWidget::AnimationReset, this, [this](){
        scene_->Reset();
    });
    connect(&animator_, &AnimationWidget::FPSChanged, this, [this](unsigned int fps){
        scene_->SetFPS(fps);
    });
    connect(&animator_, &AnimationWidget::AnilengthChanged, this, [this](unsigned int anilength){
        scene_->SetAnimationLength(anilength);
    });
    connect(&animator_, &AnimationWidget::RealtimeChanged, this, [this](bool set){
        scene_->SetRealtime(set);
    });

    // ------- Create grid -------
    AssetManager& asset_manager = scene_->GetAssetManager();
    std::vector<float> grid_positions;
    float spacing = 0.5f;
    int radius = 5;
    for (int i = -radius; i <= radius; i++) {
        grid_positions.push_back(-radius*spacing);
        grid_positions.push_back(i*spacing);
        grid_positions.push_back(0);
        grid_positions.push_back(radius*spacing);
        grid_positions.push_back(i*spacing);
        grid_positions.push_back(0);

        grid_positions.push_back(i*spacing);
        grid_positions.push_back(-radius*spacing);
        grid_positions.push_back(0);
        grid_positions.push_back(i*spacing);
        grid_positions.push_back(radius*spacing);
        grid_positions.push_back(0);
    }
    SceneObject& grid = scene_->CreateSceneObject("Grid", SceneObject::INTERNAL_HIDDEN);
    grid_ = &grid;
    grid.SetEnabled(actions_["Show Grid"]->isChecked());
    grid.AddComponent<TriangleMesh>();
    auto grid_mesh = asset_manager.CreateMesh("Grid", MeshType::Lines);
    grid.GetComponent<TriangleMesh>()->MeshFilter.Set(grid_mesh);
    Material* gray_material = asset_manager.GetMaterial("_internal Unlit Gray");
    grid.GetComponent<Geometry>()->RenderMaterial.Set(gray_material);
    grid_mesh->SetPositions(grid_positions);
    grid.GetTransform().Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90);
}

void MainWindow::CreateSplitScreen() {
    // Create split
    vsplit_ = new QSplitter(Qt::Vertical, ui->sceneWidget);
    vsplit_->setChildrenCollapsible(true);
    ui->sceneLayout->addWidget(vsplit_);
    hsplit1_ = new QSplitter(Qt::Horizontal);
    hsplit1_->setChildrenCollapsible(true);
    hsplit2_ = new QSplitter(Qt::Horizontal);
    hsplit2_->setChildrenCollapsible(true);
    vsplit_->addWidget(hsplit1_);
    vsplit_->addWidget(hsplit2_);
    connect(vsplit_, &QSplitter::splitterMoved, this, [this]() {
       CheckSplitState();
    });
    connect(hsplit1_, &QSplitter::splitterMoved, this, [this]() {
       hsplit2_->setSizes(hsplit1_->sizes());
       CheckSplitState();
    });
    connect(hsplit2_, &QSplitter::splitterMoved, this, [this]() {
       hsplit1_->setSizes(hsplit2_->sizes());
       CheckSplitState();
    });

    // Create the OpenGLWidgets
    AddSceneView(hsplit1_);
    AddSceneView(hsplit1_);
    AddSceneView(hsplit2_);
    AddSceneView(hsplit2_);
    HideSplit();
}

void MainWindow::CreateFileActions() {
    QIcon::setThemeName("FatCow");
    const QIcon new_icon = QIcon::fromTheme("picture_add");
    QAction* new_action = actions_.CreateAction("New Scene");
    new_action->setIcon(new_icon);
    new_action->setShortcuts(QKeySequence::New);
    new_action->setToolTip(tr("Create a new scene"));
    connect(new_action, &QAction::triggered, this, &MainWindow::NewScene);
    addAction(new_action);

    QAction* open_action = actions_.CreateAction("Open Scene");
    const QIcon open_icon = QIcon::fromTheme("folder");
    open_action->setIcon(open_icon);
    open_action->setShortcuts(QKeySequence::Open);
    open_action->setToolTip(tr("Open an existing scene"));
    connect(open_action, &QAction::triggered, this, &MainWindow::OpenScene);
    addAction(open_action);

    QAction* save_action = actions_.CreateAction("Save Scene");
    save_action->setIcon(QIcon::fromTheme("diskette"));
    save_action->setShortcuts(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::SaveScene);
    addAction(save_action);

    QAction* save_as_action = actions_.CreateAction("Save Scene As");
    save_as_action->setIcon(QIcon::fromTheme("diskette"));
    save_as_action->setShortcuts(QKeySequence::SaveAs);
    connect(save_as_action, &QAction::triggered, this, &MainWindow::SaveSceneAs);
    addAction(save_as_action);


    QAction* open_curve_editor_action = actions_.CreateAction("Open Curve Editor");
    connect(open_curve_editor_action, &QAction::triggered, this, [this]() {
        curve_editor_dialog_.show();
    });
    addAction(open_curve_editor_action);

}

void MainWindow::CreateRenderActions() {
    QIcon::setThemeName("FatCow");

    QAction* tracens_frame_action = actions_.CreateAction("Raytrace Frame");
    //trace_frame_action->setIcon(QIcon::fromTheme("camera-video"));
    connect(tracens_frame_action, &QAction::triggered, this, &MainWindow::RaytraceFrameNoSave);
    addAction(tracens_frame_action);

    QAction* trace_frame_action = actions_.CreateAction("Raytrace and Save Frame");
    //trace_frame_action->setIcon(QIcon::fromTheme("camera-video"));
    connect(trace_frame_action, &QAction::triggered, this, &MainWindow::RaytraceFrame);
    addAction(trace_frame_action);

    QAction* save_frames_action = actions_.CreateAction("Save Movie Frames");
    save_frames_action->setIcon(QIcon::fromTheme("camera-video"));
    connect(save_frames_action, &QAction::triggered, this, &MainWindow::RasterizeMovieFrames);
    addAction(save_frames_action);

    QAction* trace_frames_action = actions_.CreateAction("Raytrace and Save Movie Frames");
    //trace_frames_action->setIcon(QIcon::fromTheme("camera-video"));
    connect(trace_frames_action, &QAction::triggered, this, &MainWindow::RaytraceMovieFrames);
    addAction(trace_frames_action);

    QAction* trace_frame_diff_action = actions_.CreateAction("Raytrace Frame And Diff");
    connect(trace_frame_diff_action, &QAction::triggered, this, &MainWindow::RaytraceFrameAndDiff);
    addAction(trace_frame_diff_action);

    QAction* trace_frame_diff_all_action = actions_.CreateAction("Diff All Raytrace Scenes");
    connect(trace_frame_diff_all_action, &QAction::triggered, this, &MainWindow::DiffAllRaytraceScenes);
    addAction(trace_frame_diff_all_action);
}


void MainWindow::CreateViewActions() {
    QAction* split_action_ = actions_.CreateAction("Split-Screen");
    split_action_->setCheckable(true);
    split_action_->setShortcut(QKeySequence(Qt::Key_Space));
    addAction(split_action_);
    connect(split_action_, &QAction::toggled, this, [this](bool split){
        if (split) ShowSplit();
        else HideSplit();
    });

    QAction* nrm_action_ = actions_.CreateAction("Show Normals");
    nrm_action_->setCheckable(true);
    nrm_action_->setChecked(false);
    connect(nrm_action_, &QAction::toggled, this, [this](bool show) {
        for (auto& view : scene_views_) view->ShowNormals(show);
    });

    QAction* grid_action_ = actions_.CreateAction("Show Grid");
    grid_action_->setCheckable(true);
    grid_action_->setChecked(true);
    connect(grid_action_, &QAction::toggled, this, [this](bool show) {
        assert(grid_ != nullptr);
        grid_->SetEnabled(show);
        RedrawSceneViews();
    });

    QAction* sel_action_ = actions_.CreateAction("Show Selection");
    sel_action_->setCheckable(true);
    sel_action_->setChecked(true);
    connect(sel_action_, &QAction::toggled, this, [this](bool show) {
        for (auto& view : scene_views_) view->ShowSelected(show);
    });
}

void MainWindow::CreateAssetsActions() {
    QAction *create_material_action = actions_.CreateAction("Material");
    connect(create_material_action, &QAction::triggered, this, [this] {
        static int materials_created = 0;
        Material* mat = nullptr;
        while (mat == nullptr) {
            materials_created++;
            mat = scene_->GetAssetManager().CreateMaterial("Material " + std::to_string(materials_created), false);
        }
    });

    QAction *create_shader_action = actions_.CreateAction("Shader Program");
    connect(create_shader_action, &QAction::triggered, this, [this] {
        static int shaders_created = 0;
        ShaderProgram* prog = nullptr;
        while (prog == nullptr) {
            shaders_created++;
            prog = scene_->GetAssetManager().CreateShaderProgram("Shader " + std::to_string(shaders_created), false);
        }
    });

    QAction *import_mesh_action = actions_.CreateAction("Mesh", "Import Mesh");
    connect(import_mesh_action, &QAction::triggered, this, [this] {
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Mesh], 0, QFileDialog::DontUseNativeDialog);
        if (!file_name.isNull() && !file_name.isEmpty()) {
            auto file_info = QFileInfo(file_name);
            // Store current path so next time file dialog can open to here
            FilePicker::LastPath = file_info.path();
            // Use relative paths so when serialization happens, it can work when we move the program
            QDir cwd(QDir::currentPath());
            QString relative_file_name = cwd.relativeFilePath(file_name);
            // Load mesh with the basename as the mesh name by default
            scene_->GetAssetManager().LoadMesh(file_info.baseName().toStdString(), relative_file_name.toStdString());
        }
    });

    QAction *import_texture_action = actions_.CreateAction("Texture");
    connect(import_texture_action, &QAction::triggered, this, [this] {
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Image], 0, QFileDialog::DontUseNativeDialog);
        if (!file_name.isNull() && !file_name.isEmpty()) {
            auto file_info = QFileInfo(file_name);
            // Store current path so next time file dialog can open to here
            FilePicker::LastPath = file_info.path();
            // Use relative paths so when serialization happens, it can work when we move the program
            QDir cwd(QDir::currentPath());
            QString relative_file_name = cwd.relativeFilePath(file_name);
            // Load texture with the basename as the texture name by default
            scene_->GetAssetManager().LoadTexture(file_info.baseName().toStdString(), relative_file_name.toStdString());
        }
    });

    QAction *import_cubemap_action = actions_.CreateAction("Cubemap");
    connect(import_cubemap_action, &QAction::triggered, this, [this] {
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Image], 0, QFileDialog::DontUseNativeDialog);
        if (!file_name.isNull() && !file_name.isEmpty()) {
            auto file_info = QFileInfo(file_name);
            // Store current path so next time file dialog can open to here
            FilePicker::LastPath = file_info.path();
            // Use relative paths so when serialization happens, it can work when we move the program
            QDir cwd(QDir::currentPath());
            QString relative_file_name = cwd.relativeFilePath(file_name);
            // Load cubemap with the basename as the cubemap name by default
            scene_->GetAssetManager().LoadCubemap(file_info.baseName().toStdString(), relative_file_name.toStdString());
        }
    });

    QAction *export_mesh_action = actions_.CreateAction("Export Mesh", "Export Mesh");
    connect(export_mesh_action, &QAction::triggered, this, [this] {
        QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Mesh], 0, QFileDialog::DontUseNativeDialog);
        if (!file_name.isNull() && !file_name.isEmpty()) {
            if (!file_name.endsWith(".ply")) {
                file_name = file_name + ".ply";
            }
            auto file_info = QFileInfo(file_name);
            QFile file(file_name);
            if (file_info.isFile()) {
                file.remove();
            }

            if (file.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file);

                if (selected_object_ == nullptr) {
                    return;
                }
                Geometry* geo = selected_object_->GetComponent<Geometry>();
                if (geo == nullptr) {
                    return;
                }
                Mesh* mesh = geo->GetRenderMesh();
                if (mesh == nullptr) {
                    return;
                }

                const std::vector<float>& pos = mesh->GetPositions();
                const std::vector<float>& uv = mesh->GetUVs();
                const std::vector<float>& nrm = mesh->GetNormals();
                const std::vector<unsigned int>& tri = mesh->GetTriangles();

                stream << "ply\n";
                stream << "format ascii 1.0\n";
                stream << "element vertex " << pos.size()/3 << "\n";
                stream << "property float x\n";
                stream << "property float y\n";
                stream << "property float z\n";
                stream << "property float nx\n";
                stream << "property float ny\n";
                stream << "property float nz\n";
                if (uv.size() > 0) {
                    stream << "property float u\n";
                    stream << "property float v\n";
                }
                stream << "element face " << tri.size()/3 << "\n";
                stream << "property list uchar int vertex_indices\n";
                stream << "end_header\n";

                for (int i=0; i<pos.size(); i+= 3) {
                    stream << pos[i] << " " << pos[i+1] << " " << pos[i+2];
                    stream << " " << nrm[i] << " " << nrm[i+1] << " " << nrm[i+2];
                    if (uv.size() > 0) {
                        int j = (i*2)/3;
                        stream << " " << uv[j] << " " << uv[j+1];
                    }
                    stream << "\n";
                }

                for (int i=0; i<tri.size(); i+= 3) {
                    stream << "3 " << tri[i] << " " << tri[i+1] << " " << tri[i+2] << "\n";
                }

                file.close();
            }
        }
    });
    addAction(export_mesh_action);

    QAction *filter_mesh_action = actions_.CreateAction("Filter Selected");
    connect(filter_mesh_action, &QAction::triggered, this, [this] {
        if (filter_dialog_.exec()) {
            TriangleMesh* geo = selected_object_->GetComponent<TriangleMesh>();
            Mesh* mesh = geo->MeshFilter.Get();
            std::string name = "Filtered " + mesh->GetName();
            Mesh* filtered_mesh = scene_->GetAssetManager().CreateMesh(name, MeshType::Triangles, true, false);
            if (filtered_mesh == nullptr) filtered_mesh = scene_->GetAssetManager().GetMesh(name);
            for (unsigned int iteration = 0; iteration < filter_dialog_.Iterations(); iteration++) {
                MeshProcessing::FilterMesh(*mesh, *filtered_mesh, filter_dialog_.a());
                mesh = filtered_mesh;
            }
            geo->MeshFilter.Set(filtered_mesh);
            RedrawSceneViews();
        }
    });

    QAction *flip_normals_action = actions_.CreateAction("Flip Normals on Selected");
    connect(flip_normals_action, &QAction::triggered, this, [this] {
        TriangleMesh* geo = selected_object_->GetComponent<TriangleMesh>();
        Mesh* mesh = geo->MeshFilter.Get();
        std::string name = "Flipped " + mesh->GetName();
        Mesh* filtered_mesh = scene_->GetAssetManager().CreateMesh(name, MeshType::Triangles, true, false);
        if (filtered_mesh == nullptr) filtered_mesh = scene_->GetAssetManager().GetMesh(name);
        MeshProcessing::FlipNormals(*mesh, *filtered_mesh);
        geo->MeshFilter.Set(filtered_mesh);
        RedrawSceneViews();
    });

    QAction* refresh_action = actions_.CreateAction("Reload Assets");
    refresh_action->setShortcut(QKeySequence::Refresh);
    connect(refresh_action, &QAction::triggered, this, [this] {
        GrayoutConsole();
        scene_->GetAssetManager().Refresh();
    });
    addAction(refresh_action);
}

void MainWindow::CreateSceneObjectsActions() {
    QAction *create_empty_action = actions_.CreateAction("Create Empty");
    create_empty_action->setToolTip(tr("Create an empty SceneObject"));
    create_empty_action->setIcon(QIcon(":/images/icons/empty.png"));
    create_empty_action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    connect(create_empty_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateSceneObject("Empty Node " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });
    addAction(create_empty_action);

    QAction *add_envmap_action = actions_.CreateAction("Add EnvironmentMap to selected");
    add_envmap_action->setToolTip(tr("Add an EnvironmentMap component to the selected SceneObject"));
    connect(add_envmap_action, &QAction::triggered, this, [this] {
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) {
            selected_object->AddComponent<EnvironmentMap>();
        }
    });
    addAction(add_envmap_action);

    QAction *create_mesh_action = actions_.CreateAction("Mesh");
    create_mesh_action->setIcon(QIcon(":/images/icons/knot.png"));
    connect(create_mesh_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateMesh("Mesh " + std::to_string(num_nodes_created), "Cube");
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_cube_action = actions_.CreateAction("Cube");
    create_cube_action->setIcon(QIcon(":/images/icons/cube.png"));
    connect(create_cube_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateMesh("Cube " + std::to_string(num_nodes_created), "Cube");
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_sphere_action = actions_.CreateAction("Sphere");
    create_sphere_action->setIcon(QIcon(":/images/icons/sphere.png"));
    connect(create_sphere_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateSphere("Sphere " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_plane_action = actions_.CreateAction("Plane");
    create_plane_action->setIcon(QIcon(":/images/icons/plane.png"));
    connect(create_plane_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreatePlane("Plane " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_cylinder_action = actions_.CreateAction("Cylinder");
    create_cylinder_action->setIcon(QIcon(":/images/icons/cylinder.png"));
    connect(create_cylinder_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateCylinder("Cylinder " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_cone_action = actions_.CreateAction("Cone");
    create_cone_action->setIcon(QIcon(":/images/icons/cone.png"));
    connect(create_cone_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateCone("Cone " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_sof_action = actions_.CreateAction("Surface of Revolution");
    connect(create_sof_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateSurfaceOfRevolution("Surface of Revolution " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_teapot_action = actions_.CreateAction("Teapot");
    create_teapot_action->setIcon(QIcon(":/images/icons/teapot.png"));
    connect(create_teapot_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateMesh("Teapot " + std::to_string(num_nodes_created), "Teapot");
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_torus_action = actions_.CreateAction("Ring");
    create_torus_action->setIcon(QIcon(":/images/icons/torus.png"));
    connect(create_torus_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateRing("Ring " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_pointlight_action = actions_.CreateAction("Point Light");
    connect(create_pointlight_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreatePointLight("Point Light " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_dirlight_action = actions_.CreateAction("Directional Light");
    connect(create_dirlight_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateDirectionalLight("Directional Light " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_spotlight_action = actions_.CreateAction("Spot Light");
    connect(create_spotlight_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateSpotLight("Spot Light " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_arealight_action = actions_.CreateAction("Area Light (Trace only)");
    connect(create_arealight_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateAreaLight("Area Light " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });


    QAction *create_ps_action = actions_.CreateAction("Create Particle System");
    connect(create_ps_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateParticleSystem("Particle System " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_ps2_action = actions_.CreateAction("Create Fixed Particle System");
    connect(create_ps2_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateConnectedParticleSystem("Fixed Particle System " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_sphere_collider_action = actions_.CreateAction("Create Sphere Collider");
    connect(create_sphere_collider_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateSphereCollider("Sphere Collider " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_plane_collider_action = actions_.CreateAction("Create Plane Collider");
    connect(create_plane_collider_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreatePlaneCollider("Plane Collider " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *create_cylinder_collider_action = actions_.CreateAction("Create Cylinder Collider");
    connect(create_cylinder_collider_action, &QAction::triggered, this, [this] {
        static int num_nodes_created = 0;
        num_nodes_created++;
        auto new_node = scene_->CreateCylinderCollider("Cylinder Collider " + std::to_string(num_nodes_created));
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) scene_->ReparentSceneObject(new_node.GetUID(), selected_object->GetUID());
        hierarchy_.SelectObject(new_node);
    });

    QAction *add_armprop_action = actions_.CreateAction("Add Robot Arm Property to selected");
    add_armprop_action->setToolTip(tr("Add a Robot Arm Property to the selected SceneObject"));
    connect(add_armprop_action, &QAction::triggered, this, [this] {
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) {
            selected_object->AddComponent<RobotArmProp>().SetRoot(selected_object);
        }
    });
    addAction(add_armprop_action);

    QAction *add_customprop_action = actions_.CreateAction("Add Customized Property to selected");
    add_customprop_action->setToolTip(tr("Add a Customized Property to the selected SceneObject"));
    connect(add_customprop_action, &QAction::triggered, this, [this] {
        SceneObject* selected_object = hierarchy_.GetSelectedSceneObject();
        if (selected_object) {
            selected_object->AddComponent<CustomProp>().SetRoot(selected_object);
        }
    });
    addAction(add_customprop_action);
}

void MainWindow::CreateManipulatorActions() {
    // Select
    QAction* select_action = actions_.CreateAction("Select");
    select_action->setIcon(QIcon::fromTheme("cursor"));
    select_action->setCheckable(true);
    select_action->setShortcut(QKeySequence(Qt::Key_Q));
    select_action->setToolTip(tr("Return to default mouse mode"));
    connect(select_action, &QAction::triggered, this, [this](){
        mode = SceneWindow::MOUSEMODE_CAMERA;
        for (auto view : scene_views_) view->SetMode(mode);
    });
    select_action->setChecked(true);
    addAction(select_action);

    // Translate
    QAction* translate_action = actions_.CreateAction("Translate");
    translate_action->setIcon(QIcon::fromTheme("transform_move"));
    translate_action->setCheckable(true);
    translate_action->setShortcut(QKeySequence(Qt::Key_W));
    translate_action->setToolTip(tr("Translate the selected object"));
    connect(translate_action, &QAction::triggered, this, [=](){
        if (mode != SceneWindow::MOUSEMODE_TRANSLATE) {
            mode = SceneWindow::MOUSEMODE_TRANSLATE;
            for (auto view : scene_views_) view->SetMode(mode);
        } else {
            select_action->trigger();
        }
    });
    addAction(translate_action);

    // Rotate
    QAction* rotate_action = actions_.CreateAction("Rotate");
    rotate_action->setIcon(QIcon::fromTheme("transform_rotate"));
    rotate_action->setCheckable(true);
    rotate_action->setShortcut(QKeySequence(Qt::Key_E));
    rotate_action->setToolTip(tr("Rotate the selected object"));
    connect(rotate_action, &QAction::triggered, this, [=](){
        if (mode != SceneWindow::MOUSEMODE_ROTATE) {
            mode = SceneWindow::MOUSEMODE_ROTATE;
            for (auto view : scene_views_) view->SetMode(mode);
        } else {
            select_action->trigger();
        }
    });
    addAction(rotate_action);

    // Scale
    QAction* scale_action = actions_.CreateAction("Scale");
    scale_action->setIcon(QIcon::fromTheme("transform_scale"));
    scale_action->setCheckable(true);
    scale_action->setShortcut(QKeySequence(Qt::Key_R));
    scale_action->setToolTip(tr("Scale the selected object"));
    connect(scale_action, &QAction::triggered, this, [=](){
        if (mode != SceneWindow::MOUSEMODE_SCALE) {
            mode = SceneWindow::MOUSEMODE_SCALE;
            for (auto view : scene_views_) view->SetMode(mode);
        } else {
            select_action->trigger();
        }
    });
    addAction(scale_action);

    // Local / World Space toggle
    QAction* localspace_action = actions_.CreateAction("Local");
    QAction* worldspace_action = actions_.CreateAction("World");
    connect(localspace_action, &QAction::triggered, this, [this]() {
        for (auto view : scene_views_) {
            view->SetManipulationSpace(Space::Local);
            view->Redraw();
        }
    });
    connect(worldspace_action, &QAction::triggered, this, [this]() {
        for (auto view : scene_views_) {
            view->SetManipulationSpace(Space::World);
            view->Redraw();
        }
    });

    QAction *vertex_edit_action = actions_.CreateAction("VertexEdit");
    vertex_edit_action->setIcon(QIcon(":/images/icons/vertexedit.png"));
    vertex_edit_action->setCheckable(true);
    vertex_edit_action->setToolTip(tr("Toggle vertex edit"));
    connect(vertex_edit_action, &QAction::triggered, this, [=](){
        vertex_editing = !vertex_editing;
        for (auto view : scene_views_) {
            view->SetVertexEditing(vertex_editing);
            view->Redraw();
        }
        actions_["VertexEdit"]->setChecked(vertex_editing);
    });
    addAction(vertex_edit_action);
}

void MainWindow::CreateConsoleToolbar() {
    QAction* clear_action = new QAction("Clear Console", ui->consoleWidget);
    clear_action->setIcon(QIcon::fromTheme("bin_empty"));
    connect(clear_action, &QAction::triggered, this, [=]() {
            ui->console->clear();
    });

    QAction* copy_action = new QAction("Copy Console Contents", ui->consoleWidget);
    copy_action->setIcon(QIcon::fromTheme("clipboard_sign"));
    connect(copy_action, &QAction::triggered, this, [=]() {
            ui->console->selectAll();
            ui->console->copy();
    });
    QToolBar* toolbar = new QToolBar(ui->consoleWidget);
    toolbar->addAction(clear_action);
    toolbar->addAction(copy_action);
    toolbar->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    toolbar->setIconSize(QSize(16,16));
    ui->consoleWidget->layout()->addWidget(toolbar);
}

void MainWindow::CreateToolbar() {
    // Select / Translate / Rotate / Scale Tool Group
    QActionGroup* manipulation_actions = new QActionGroup(this);
    manipulation_actions->addAction(actions_["Select"]);
    manipulation_actions->addAction(actions_["Translate"]);
    manipulation_actions->addAction(actions_["Rotate"]);
    manipulation_actions->addAction(actions_["Scale"]);

    // Local / World Space Tool
    QMenu* manipulation_space_menu = new QMenu("Coordinate Space");
    manipulation_space_menu->addAction(actions_["Local"]);
    manipulation_space_menu->addAction(actions_["World"]);
    MenuToolButton* manipulation_space_tool = new MenuToolButton();
    manipulation_space_tool->setMenu(manipulation_space_menu);
    manipulation_space_tool->setDefaultAction(actions_["Local"]);

    // Vertex edit
    QActionGroup* manipulation_editor_actions = new QActionGroup(this);
    manipulation_editor_actions->addAction(actions_["VertexEdit"]);

    // Toolbar
    QToolBar* toolbar = new QToolBar(this);
    toolbar->addAction(actions_["Open Scene"]);
    toolbar->addAction(actions_["Save Scene"]);
    toolbar->addSeparator();
    toolbar->addWidget(new QLabel(tr(" Manipulation Mode ")));
    toolbar->addActions(manipulation_actions->actions());
    toolbar->addWidget(manipulation_space_tool);
    toolbar->addSeparator();
    toolbar->addActions(manipulation_editor_actions->actions());
    toolbar->addSeparator();
    toolbar->addAction(actions_["Create Empty"]);
    toolbar->addAction(actions_["Mesh"]);
    toolbar->addAction(actions_["Cube"]);
    toolbar->addAction(actions_["Sphere"]);
    toolbar->addAction(actions_["Cylinder"]);
    toolbar->addAction(actions_["Plane"]);
    toolbar->addAction(actions_["Cone"]);
    toolbar->addAction(actions_["Teapot"]);
    toolbar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolbar);
}

void MainWindow::CreateMenus() {
    QMenuBar* menuBar = this->menuBar();

    // File Menu
    QMenu* file_menu_ = menuBar->addMenu(tr("File"));
    file_menu_->setToolTipsVisible(true);
    file_menu_->addAction(actions_["New Scene"]);
    file_menu_->addAction(actions_["Open Scene"]);
    file_menu_->addSeparator();
    file_menu_->addAction(actions_["Save Scene"]);
    file_menu_->addAction(actions_["Save Scene As"]);
    file_menu_->addSeparator();
    file_menu_->addAction(actions_["Open Curve Editor"]);

    QMenu* render_menu_ = menuBar->addMenu(tr("Render"));
    render_menu_->addAction(actions_["Raytrace Frame"]);
    render_menu_->addAction(actions_["Raytrace and Save Frame"]);
    render_menu_->addAction(actions_["Save Movie Frames"]);
    render_menu_->addAction(actions_["Raytrace and Save Movie Frames"]);
    render_menu_->addSeparator();
    render_menu_->addAction(actions_["Raytrace Frame And Diff"]);
    render_menu_->addAction(actions_["Diff All Raytrace Scenes"]);

    // View Menu
    QMenu* view_menu_ = menuBar->addMenu(tr("View"));
    view_menu_->setToolTipsVisible(true);
    view_menu_->addAction(actions_["Split-Screen"]);
    view_menu_->addAction(actions_["Show Grid"]);
    view_menu_->addAction(actions_["Show Normals"]);
    view_menu_->addAction(actions_["Show Selection"]);

    // Assets Menu
    QMenu* asset_menu_ = menuBar->addMenu(tr("Assets"));
    asset_menu_->setToolTipsVisible(true);
    QMenu* asset_create_menu_ = asset_menu_->addMenu(tr("Create"));
    asset_create_menu_->addAction(actions_["Material"]);
    asset_create_menu_->addAction(actions_["Shader Program"]);
    QMenu* asset_import_menu_ = asset_menu_->addMenu(tr("Import"));
    asset_import_menu_->addAction(actions_["Import Mesh"]);
    asset_import_menu_->addAction(actions_["Texture"]);
    asset_import_menu_->addAction(actions_["Cubemap"]);
    asset_menu_->addAction(actions_["Export Mesh"]);
    QMenu* asset_process_mesh_menu_ = asset_menu_->addMenu(tr("Mesh Processing"));
    asset_process_mesh_menu_->addAction(actions_["Filter Selected"]);
    asset_process_mesh_menu_->addAction(actions_["Flip Normals on Selected"]);
    asset_menu_->addAction(actions_["Reload Assets"]);

    // Scene Objects Menu
    QMenu* object_menu_ = menuBar->addMenu(tr("SceneObject"));
    object_menu_->setToolTipsVisible(true);
    object_menu_->addAction(actions_["Create Empty"]);
    object_menu_->addAction(actions_["Add EnvironmentMap to selected"]);
    QMenu* object_3d_menu_ = object_menu_->addMenu(tr("Create 3D Object"));
    object_3d_menu_->addAction(actions_["Mesh"]);
    object_3d_menu_->addAction(actions_["Cube"]);
    object_3d_menu_->addAction(actions_["Sphere"]);
    object_3d_menu_->addAction(actions_["Plane"]);
    object_3d_menu_->addAction(actions_["Cylinder"]);
    object_3d_menu_->addAction(actions_["Cone"]);
    object_3d_menu_->addAction(actions_["Teapot"]);
    object_3d_menu_->addAction(actions_["Surface of Revolution"]);
    QMenu* object_lights_menu_ = object_menu_->addMenu(tr("Create Light Source"));
    object_lights_menu_->addAction(actions_["Point Light"]);
    object_lights_menu_->addAction(actions_["Directional Light"]);
    object_lights_menu_->addAction(actions_["Spot Light"]);
    object_lights_menu_->addAction(actions_["Area Light (Trace only)"]);
    object_menu_->addAction(actions_["Create Particle System"]);
    object_menu_->addAction(actions_["Create Fixed Particle System"]);
    QMenu* object_colliders_menu_ = object_menu_->addMenu(tr("Create Collider"));
    object_colliders_menu_->addAction(actions_["Create Sphere Collider"]);
    object_colliders_menu_->addAction(actions_["Create Plane Collider"]);
    object_colliders_menu_->addAction(actions_["Create Cylinder Collider"]);
    object_menu_->addSeparator();
    object_menu_->addAction(actions_["Add Robot Arm Property to selected"]);
    object_menu_->addAction(actions_["Add Customized Property to selected"]);

    hierarchy_context_menu_->addAction(actions_["Create Empty"]);
    hierarchy_context_menu_->addMenu(object_3d_menu_);
    hierarchy_context_menu_->addMenu(object_lights_menu_);
}
