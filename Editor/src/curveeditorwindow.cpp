/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "curveeditorwindow.h"
#include "ui_curveeditorwindow.h"
#include <widgets/filepicker.h>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <string>
#include <animation/beziercurveevaluator.h>
#include <animation/catmullromcurveevaluator.h>
#include <animation/linearcurveevaluator.h>

CurveEditorWindow::CurveEditorWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurveEditorWindow),
    selected_index_(-1),
    needs_save_(false),
    save_scale_(0.25,-0.25)
{
    ui->setupUi(this);
    setWindowTitle("Curve Editor");

    // Initialize combo box
    ui->interpolationComboBox->addItem("Catmull-Rom");
    ui->interpolationComboBox->addItem("Linear");
    //ui->interpolationComboBox->addItem("C2");
    // Initialize canvas
    connect(ui->CurveDrawingArea, &CurveEditorCanvas::PointCreated, this, [this](float x, float y, int idx) {
        points_.insert(points_.begin()+idx, glm::vec2(x, y));
        selected_index_ = idx;
        needs_save_ = true;
        Redraw();
    });
    connect(ui->CurveDrawingArea, &CurveEditorCanvas::PointDeleted, this, [this](int idx) {
        points_.erase(points_.begin()+idx);
        if (points_.size() > 1) needs_save_ = true;
        Redraw();
    });
    connect(ui->CurveDrawingArea, &CurveEditorCanvas::PointMoved, this, [this](float x, float y, int idx) {
        points_[idx] = glm::vec2(x, y);
        needs_save_ = true;
        Redraw();
    });
    connect(ui->CurveDrawingArea, &CurveEditorCanvas::PointSelected, this, [this](int idx) {
        selected_index_ = idx;
        Redraw();
    });

    // Initialize actions
    connect(ui->densitySlider, &QSlider::sliderMoved, this, [this](int) {
        if (points_.size() > 1) needs_save_ = true;
        Redraw();
    });
    connect(ui->interpolationComboBox, &QComboBox::currentTextChanged, this, [this](QString) {
        needs_save_ = true;
        Redraw();
    });
    connect(ui->saveDenseCurveButton, &QPushButton::clicked, this, [this]() {
        MakePointsRightHanded();
        if (points_.size() < 1) return;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Dense Points"), FilePicker::LastPath, FilePicker::FileFilters[FileType::Points], 0, QFileDialog::DontUseNativeDialog);
        if (filename.isNull() || filename.isEmpty()) return;
        if (!filename.endsWith(".apts")) filename += ".apts";
        std::vector<glm::vec2> densepoints;
        int sample_density = ui->densitySlider->value();
        ComputeDensePoints(densepoints, sample_density);
        CleanupDensePoints(densepoints);
        std::ofstream out(filename.toStdString());
        for (size_t i = 0; i < densepoints.size(); i++) {
            out << densepoints[i].x*save_scale_.x << " ";
            out << densepoints[i].y*save_scale_.y << std::endl;
        }
    });
    connect(ui->saveButton, &QPushButton::clicked, this, &CurveEditorWindow::SavePoints);
    connect(ui->loadButton, &QPushButton::clicked, this, [this]() {
        points_.clear();
        QString filename = QFileDialog::getOpenFileName(this, tr("Save Dense Points"), FilePicker::LastPath, "Text Files (*.txt)", 0, QFileDialog::DontUseNativeDialog);
        if (filename.isNull() || filename.isEmpty()) return;
        std::ifstream in(filename.toStdString());
        double x, y;
        try {
            while (in >> x >> y) {
                points_.push_back(glm::vec2(x,y));
            }
        } catch (std::exception const& e) {
            QMessageBox::warning(this, "Invalid Points File", "Invalid points file!");
        }
        std::ifstream cfgin(filename.toStdString() + ".cfg");
        int sample_density;
        std::string interp;
        try {
            cfgin >> sample_density;
            sample_density = std::min(std::max(sample_density, ui->densitySlider->minimum()), ui->densitySlider->maximum());
            std::getline(cfgin, interp);
            std::getline(cfgin, interp);
            ui->interpolationComboBox->setCurrentText(QString::fromStdString(interp));
            ui->densitySlider->setValue(sample_density);
        } catch (std::exception const& e) {
            QMessageBox::warning(this, "Invalid Points Config File", "Invalid points config file!");
        }
        Redraw();
        selected_index_ = -1;
    });
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() {
        points_.clear();
        Redraw();
    });
    connect(ui->wrapCheckbox, &QCheckBox::toggled, this, [this](bool) {
       needs_save_ = true;
       Redraw();
    });
    Redraw();
}

CurveEditorWindow::~CurveEditorWindow()
{
    delete ui;
}

void CurveEditorWindow::Redraw() {
    ui->CurveDrawingArea->SetSelectedIndex(selected_index_);

    std::vector<glm::vec2> densepoints;
    int sample_density = ui->densitySlider->value();
    ComputeDensePoints(densepoints, sample_density);
    ui->CurveDrawingArea->UpdatePoints(densepoints);
    ui->CurveDrawingArea->UpdateControlPoints(points_);
}

void CurveEditorWindow::closeEvent(QCloseEvent *event) {
    if (needs_save_) {
        QMessageBox::StandardButton btn = QMessageBox::question(this, "Curve Modified", "The curve has been modified. Save changes?",
                                                                QMessageBox::Ok | QMessageBox::No | QMessageBox::Cancel);
        if (btn == QMessageBox::Cancel) {
            event->ignore();
            return;
        } else if (btn == QMessageBox::Ok) {
            SavePoints();
        }
        points_.clear();
        ui->densitySlider->setValue(10);
        Redraw();
        event->accept();
    } else {
        points_.clear();
        ui->densitySlider->setValue(10);
        Redraw();
        event->accept();
    }
}

void CurveEditorWindow::SavePoints() {
    if (points_.size() < 1) return;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Dense Points"), FilePicker::LastPath, "Text Files (*.txt)", 0, QFileDialog::DontUseNativeDialog);
    if (filename.isNull() || filename.isEmpty()) return;
    if (!filename.endsWith(".txt")) filename += ".txt";
    std::ofstream out(filename.toStdString());
    for (size_t i = 0; i < points_.size(); i++) {
        out << points_[i].x << " ";
        out << points_[i].y << std::endl;
    }
    std::ofstream cfgout(filename.toStdString() + ".cfg");
    int sample_density = ui->densitySlider->value();
    cfgout << sample_density << std::endl;
    cfgout << ui->interpolationComboBox->currentText().toStdString() << std::endl;
    needs_save_ = false;
}

//Does the ray from Point A to +ve infinity intersect line segment (B1,B2)
bool Intersect(glm::vec2 A, glm::vec2 B1, glm::vec2 B2 )
{
    if ((B1.y > A.y && B2.y > A.y) || (B1.y < A.y && B2.y < A.y))
        return false;
    else if(B1.y < A.y) {
        if (B1.x + (B2.x - B1.x) * (A.y - B1.y)/(B2.y - B1.y) > A.x)
            return true;
        else
            return false;
    }
    else {
        if (B2.x + (B1.x - B2.x) * (A.y - B2.y) / (B1.y - B2.y) > A.x)
            return true;
        else
            return false;
    }
}
void CurveEditorWindow::ComputeDensePoints(std::vector<glm::vec2>& dense_points, int sample_rate) {
    std::vector<glm::vec2> sub_points(points_); // Copy points so we can insert points if we're wrapping without editing the persisted state
    dense_points.clear();
    if (points_.size() <= 1 || sample_rate <= 0) return;
    if (sample_rate == 1) {
        dense_points.resize(points_.size());
        std::copy(points_.begin(), points_.end(), dense_points.begin());
        return;
    }
    bool wrap = ui->wrapCheckbox->isChecked();
    std::string Interpolation = ui->interpolationComboBox->currentText().toStdString();

    if (Interpolation == "C2") {
        // FIXME: Unimplemented
    } else if (Interpolation == "Linear") {
        if (wrap) {
            sub_points.push_back(sub_points[0]);
        }
        LinearCurveEvaluator curveevaluator;
        std::vector<glm::vec2> pts = curveevaluator.EvaluateCurve(sub_points, sample_rate);
        pts.swap(dense_points);
    } else if (Interpolation == "Catmull-Rom") {
        CatmullRomCurveEvaluator curveevaluator;
        if (wrap) curveevaluator.Wrap();
        std::vector<glm::vec2> pts = curveevaluator.EvaluateCurve(sub_points, sample_rate);
        pts.swap(dense_points);
    }
}

void CurveEditorWindow::CleanupDensePoints(std::vector<glm::vec2>& dense_points) {
    std::vector<glm::vec2> sub_points(points_); // Copy points so we can insert points if we're wrapping without editing the persisted state
    int num_points = sub_points.size();
    // compute which of these dense points is outward facing, by checking if the ray from each point to +ve infinity on the x axis
    // intersects the curve. If it doesn't intersect any segment, then the point is an Extrema.
    int countExtrema = dense_points.size();
    std::vector<bool> Extrema(countExtrema);
    for (int i = 0; i < dense_points.size(); ++i) {
        glm::vec2 Point1 = dense_points[i];
        Extrema[i] = true;
        for (int j = 0; j < num_points-1; j++ ) {
            glm::vec2 Point3 = sub_points[j];
            glm::vec2 Point4 = sub_points[j + 1];
            if (Intersect(Point1, Point3, Point4)) {
                Extrema[i] = false;
                countExtrema--;
                break;
            }
        }
    }

    // Check if the computed normal at each extrema point is pointing outward, and count the number of such points.
    int countOutward = 0;
    for (int i = 1; i < dense_points.size(); ++i) {
        if (Extrema[i]) {
            glm::vec2 Ray = dense_points[i] - dense_points[i-1];
            if (Ray.y > 0) countOutward++;
        }
    }

    // check if more than 50% points having outward facing normals
    if (countOutward <= 0.5 * countExtrema-1) {
        std::reverse(dense_points.begin(), dense_points.end());
    }
}

void CurveEditorWindow::MakePointsRightHanded() {
    // If most of the points on the curve are on the left side (x < 0),
    // we'll assume the curve was drawn "left-handed." Because this
    // can make for inconsistent surface of revolution drawing
    // calculations, we'll flip the curve such that it's now
    // "right-handed" by reversing the sign of its x-coordinates.
    int posCounter = 0;
    int negCounter = 0;
    for (size_t i = 0; i < points_.size(); i++)  {
        if (points_[i].x >= 0) posCounter++;
        else                   negCounter++;
    }
    if (posCounter < negCounter) {
        // Flip "handedness" of curve
        for (size_t i = 0; i < points_.size(); i++)  {
            points_[i] = glm::vec2(-points_[i].x, points_[i].y);
        }
        Redraw();
    }
}
