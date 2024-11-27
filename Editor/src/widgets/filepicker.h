/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef FILEPICKER_H
#define FILEPICKER_H

#include <QWidget>
#include <widgets/inspectablewidget.h>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class FilePicker : public QWidget, public InspectableWidget {
    Q_OBJECT

public:
    static std::map<FileType, QString> FileFilters;
    static QString LastPath;
    Signal1<std::string> Changed;

    FilePicker(FileType type, const std::string& path = std::string(), QWidget *parent = Q_NULLPTR);

public slots:
    void OnSetValue(std::string path);

protected:
    QHBoxLayout* layout_;
    QLineEdit* display_;
    QPushButton* dialog_btn_;
    QString path_;
    FileType type_;
};


#endif // FILEPICKER_H
