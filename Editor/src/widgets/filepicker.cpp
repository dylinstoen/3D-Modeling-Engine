/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "filepicker.h"
#include <QFileInfo>
#include <QFileDialog>

QString FilePicker::LastPath = QDir::currentPath();
std::map<FileType, QString> FilePicker::FileFilters = {
    {FileType::VertexShader, QString::fromStdString("Vertex Shaders (*.vert)")},
    {FileType::FragmentShader, QString::fromStdString("Fragment Shaders (*.frag)")},
    {FileType::GeometryShader, QString::fromStdString("Geometry Shaders (*.geom)")},
    {FileType::Curve, QString::fromStdString("Curve Files (*.apts)")},
    {FileType::Image, QString::fromStdString("Image Files (*.jpg *.jpeg *.png *.bmp *.tga *.psd)")},
    {FileType::Mesh, QString::fromStdString("Mesh Files (*.obj *.ply *.stl)")},
    {FileType::Scene, QString::fromStdString("Scene Files (*.yaml)")},
    {FileType::Points, QString::fromStdString("Point Sample Files (*.apts)")},
    {FileType::Ray, QString::fromStdString("Ray File (*.ray)")}
};

FilePicker::FilePicker(FileType type, const std::string& path, QWidget *parent) :
    QWidget(parent),
    layout_(new QHBoxLayout()),
    display_(new QLineEdit()),
    dialog_btn_(new QPushButton()),
    path_(QString::fromStdString(path)),
    type_(type)
{
    setLayout(layout_);
    layout_->setMargin(0);
    layout_->addWidget(display_);
    layout_->addWidget(dialog_btn_);
    display_->setDisabled(true);
    display_->setText(QString::fromStdString(path));

    // dialog_btn_->setFlat(true);
    dialog_btn_->setText(QString("..."));
    dialog_btn_->setMaximumWidth(25);

    connect(dialog_btn_, &QPushButton::clicked, this, [this] {
        // Bug: If we try to capture the variable "type" instead of having a field "type_", type is always 0.
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), path_, FilePicker::FileFilters[type_], 0, QFileDialog::DontUseNativeDialog);
        if (!file_name.isNull() && !file_name.isEmpty()) {
            path_ = QFileInfo(file_name).path();
            // Store current path so next time file dialog can open to here
            LastPath = path_;
            // Use relative paths so when serialization happens, it can work when we move the program
            QDir cwd(QDir::currentPath());
            QString relative_file_name = cwd.relativeFilePath(file_name);
            Changed.Emit(relative_file_name.toStdString());
        }
    });
}

void FilePicker::OnSetValue(std::string path) {
    display_->setText(QString::fromStdString(path));
    RedrawRequested.Emit();
}


