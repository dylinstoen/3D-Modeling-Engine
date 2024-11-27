/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <properties.h>
#include <QMap>
#include <QDialog>

namespace Ui {
class PropertiesDialog;
}

class PropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PropertiesDialog(QWidget *parent = 0);
    ~PropertiesDialog();

    virtual int exec(const ObjectWithProperties& properties);
signals:
    void PropertyChanged(Property& prop, bool hidden);
protected:
    Ui::PropertiesDialog *ui;
    QMap<QString, Property*> properties_;
};

#endif // PROPERTIESDIALOG_H
